#!/usr/bin/env python3
"""Scenario 1 fairness/acceptance checker + optional QERS runner.

Workflow:
- Run analyze_campaign to regenerate summaries/KPI from joined S1 CSVs.
- Evaluate acceptance criteria (session/reconn) from summary tables.
- Emit CSV/MD reports and (optionally) compute QERS per-mode.

Usage example:
  python3 tools/check_fairness.py \
    --s1-joined-dir data/csv_joined_latest \
    --out-dir data/analysis/fairness_latest \
    --drop-warmup 5 \
    --run-qers \
    --w-sec "Pr=0.5,K=0.2,R=0.1,Co=0.2"
"""
from __future__ import annotations

import argparse
import csv
import math
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple

ANALYZE_SCRIPT = Path(__file__).resolve().parent / "analyze_campaign.py"
QERS_SCRIPT = Path(__file__).resolve().parent / "compute_qers.py"


@dataclass
class Stat:
    n: int
    mean: float
    sd: float
    cv_pct: float | None
    p50: float
    p95: float
    min: float
    max: float


@dataclass
class Quality:
    n_total: int
    n_success: int
    packet_loss_rate: float
    ss_valid_rate_success: float


@dataclass
class CheckResult:
    variant: str
    pie: str
    mode: str
    check: str
    status: str  # PASS/FAIL
    value: str
    threshold: str
    note: str


def read_summary(summary_csv: Path) -> Dict[Tuple[str, str, str, str], Stat]:
    stats: Dict[Tuple[str, str, str, str], Stat] = {}
    with summary_csv.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            key = (
                (row.get("variant") or "").strip(),
                (row.get("pie") or "").strip(),
                (row.get("mode") or "").strip(),
                (row.get("metric") or "").strip(),
            )
            n = int(row["n"])
            mean = float(row["mean"])
            sd = float(row["sd"])
            cv_raw = row.get("cv_pct", "")
            cv_pct = float(cv_raw) if cv_raw not in {"", None} else None
            p50 = float(row["p50"])
            p95 = float(row["p95"])
            min_v = float(row["min"])
            max_v = float(row["max"])
            stats[key] = Stat(n=n, mean=mean, sd=sd, cv_pct=cv_pct, p50=p50, p95=p95, min=min_v, max=max_v)
    return stats


def read_quality(quality_csv: Path) -> Dict[Tuple[str, str, str], Quality]:
    qmap: Dict[Tuple[str, str, str], Quality] = {}
    with quality_csv.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            key = (
                (row.get("variant") or "").strip(),
                (row.get("pie") or "").strip(),
                (row.get("mode") or "").strip(),
            )
            n_total = int(row["n_total"])
            n_success = int(row["n_success"])
            plr = float(row["packet_loss_rate"])
            ss_rate = float(row["ss_valid_rate_success"])
            qmap[key] = Quality(n_total=n_total, n_success=n_success, packet_loss_rate=plr, ss_valid_rate_success=ss_rate)
    return qmap


def ratio(x: float, y: float) -> float:
    if y == 0:
        return math.inf
    return x / y


def fmt(v: float | None) -> str:
    if v is None or math.isnan(v):
        return ""
    if abs(v - round(v)) < 1e-9:
        return str(int(round(v)))
    return f"{v:.3f}"


def add_check(results: List[CheckResult], variant: str, pie: str, mode: str, check: str, ok: bool, value: str, threshold: str, note: str = ""):
    results.append(
        CheckResult(
            variant=variant,
            pie=pie,
            mode=mode,
            check=check,
            status="PASS" if ok else "FAIL",
            value=value,
            threshold=threshold,
            note=note,
        )
    )


def evaluate_session(variant: str, pie: str, stats: Dict[Tuple[str, str, str, str], Stat], quality: Quality, args, results: List[CheckResult]):
    key = (variant, pie, "session")
    # Basic quality
    add_check(results, *key, "packet_loss_rate", quality.packet_loss_rate == 0, fmt(quality.packet_loss_rate), "== 0")
    add_check(results, *key, "ss_valid_rate_success", quality.ss_valid_rate_success == 1, fmt(quality.ss_valid_rate_success), "== 1")

    def metric(name: str) -> Stat | None:
        return stats.get((variant, pie, "session", name))

    rssi = metric("rssi_dbm")
    if rssi:
        rssi_ok = (args.rssi_target - args.rssi_tolerance) <= rssi.p50 <= (args.rssi_target + args.rssi_tolerance)
        add_check(results, *key, "rssi_dbm_p50", rssi_ok, fmt(rssi.p50), f"{args.rssi_target}±{args.rssi_tolerance}", "IQR not available; only p50 checked")

    net = metric("net_latency_us")
    if net:
        cv_ok = (net.cv_pct or 0) <= args.net_cv_max
        ratio_ok = ratio(net.p95, net.p50) <= args.net_ratio_max
        add_check(results, *key, "net_latency_cv_pct", cv_ok, fmt(net.cv_pct), f"<= {args.net_cv_max}")
        add_check(results, *key, "net_latency_p95/p50", ratio_ok, f"{ratio(net.p95, net.p50):.3f}", f"<= {args.net_ratio_max}")

    jitter = metric("jitter_us")
    if jitter and net:
        jitter_abs_ok = jitter.p95 < args.jitter_abs_max
        jitter_rel_ok = jitter.p95 < args.jitter_ratio_max * net.p50
        add_check(
            results,
            *key,
            "jitter_us_p95_abs",
            jitter_abs_ok,
            fmt(jitter.p95),
            f"< {args.jitter_abs_max}",
            "OR relative criterion",
        )
        add_check(
            results,
            *key,
            "jitter_us_p95_rel",
            jitter_rel_ok,
            fmt(jitter.p95),
            f"< {args.jitter_ratio_max} × net_latency_p50 ({net.p50:.0f})",
            "Relative criterion",
        )

    energy = metric("energy_uj")
    if energy:
        cv_ok = (energy.cv_pct or 0) <= args.energy_cv_max
        ratio_ok = ratio(energy.p95, energy.p50) <= args.energy_ratio_max
        add_check(results, *key, "energy_uj_cv_pct", cv_ok, fmt(energy.cv_pct), f"<= {args.energy_cv_max}")
        add_check(results, *key, "energy_uj_p95/p50", ratio_ok, f"{ratio(energy.p95, energy.p50):.3f}", f"<= {args.energy_ratio_max}")

    cpu = metric("cpu_util_percent")
    if cpu:
        add_check(results, *key, "cpu_util_cv_pct", (cpu.cv_pct or 0) <= args.cpu_cv_max, fmt(cpu.cv_pct), f"<= {args.cpu_cv_max}")

    keygen = metric("keygen_us")
    if keygen:
        add_check(results, *key, "keygen_cv_pct", (keygen.cv_pct or 0) <= args.keygen_cv_max, fmt(keygen.cv_pct), f"<= {args.keygen_cv_max}")

    decaps = metric("decaps_us")
    if decaps:
        add_check(results, *key, "decaps_cv_pct", (decaps.cv_pct or 0) <= args.decaps_cv_max, fmt(decaps.cv_pct), f"<= {args.decaps_cv_max}")

    encaps = metric("encaps_us")
    if encaps:
        cv_ok = (encaps.cv_pct or 0) <= args.encaps_cv_max
        ratio_ok = ratio(encaps.p95, encaps.p50) <= args.encaps_ratio_max
        add_check(results, *key, "encaps_cv_pct", cv_ok, fmt(encaps.cv_pct), f"<= {args.encaps_cv_max}")
        add_check(results, *key, "encaps_p95/p50", ratio_ok, f"{ratio(encaps.p95, encaps.p50):.3f}", f"<= {args.encaps_ratio_max}")


def evaluate_reconn(variant: str, pie: str, stats: Dict[Tuple[str, str, str, str], Stat], quality: Quality, args, results: List[CheckResult]):
    key = (variant, pie, "reconn")
    add_check(results, *key, "packet_loss_rate", quality.packet_loss_rate == 0, fmt(quality.packet_loss_rate), "== 0")
    add_check(results, *key, "ss_valid_rate_success", quality.ss_valid_rate_success == 1, fmt(quality.ss_valid_rate_success), "== 1")

    def metric(name: str) -> Stat | None:
        return stats.get((variant, pie, "reconn", name))

    rssi = metric("rssi_dbm")
    if rssi:
        rssi_ok = (args.rssi_target - args.rssi_tolerance) <= rssi.p50 <= (args.rssi_target + args.rssi_tolerance)
        add_check(results, *key, "rssi_dbm_p50", rssi_ok, fmt(rssi.p50), f"{args.rssi_target}±{args.rssi_tolerance}", "IQR not available; only p50 checked")

    reconnect = metric("reconnect_us")
    if reconnect:
        target_min = args.reconnect_target_us * (1 - args.reconnect_tol_pct / 100.0)
        target_max = args.reconnect_target_us * (1 + args.reconnect_tol_pct / 100.0)
        in_band = target_min <= reconnect.p50 <= target_max
        cv_ok = (reconnect.cv_pct or 0) <= args.reconnect_cv_max
        ratio_ok = ratio(reconnect.p95, reconnect.p50) <= args.reconnect_ratio_max
        add_check(results, *key, "reconnect_us_p50", in_band, fmt(reconnect.p50), f"{target_min:.0f}..{target_max:.0f}")
        add_check(results, *key, "reconnect_us_cv_pct", cv_ok, fmt(reconnect.cv_pct), f"<= {args.reconnect_cv_max}")
        add_check(results, *key, "reconnect_us_p95/p50", ratio_ok, f"{ratio(reconnect.p95, reconnect.p50):.3f}", f"<= {args.reconnect_ratio_max}")

    reconn_energy = metric("reconnect_energy_uj")
    if reconn_energy:
        add_check(results, *key, "reconnect_energy_cv_pct", (reconn_energy.cv_pct or 0) <= args.reconnect_energy_cv_max, fmt(reconn_energy.cv_pct), f"<= {args.reconnect_energy_cv_max}")

    # Sanity: handshake_us should be similar to session, but we skip cross-mode compare here.


def write_checks(out_dir: Path, results: List[CheckResult]):
    fields = ["variant", "pie", "mode", "check", "status", "value", "threshold", "note"]
    out_csv = out_dir / "fairness_checks.csv"
    out_md = out_dir / "fairness_checks.md"

    out_dir.mkdir(parents=True, exist_ok=True)
    with out_csv.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        for r in results:
            writer.writerow(r.__dict__)

    # Markdown table
    widths = {f: len(f) for f in fields}
    for r in results:
        for f in fields:
            widths[f] = max(widths[f], len(str(getattr(r, f))))

    def row_line(r: CheckResult) -> str:
        return "| " + " | ".join(str(getattr(r, f)).ljust(widths[f]) for f in fields) + " |"

    header = "| " + " | ".join(f.ljust(widths[f]) for f in fields) + " |"
    sep = "| " + " | ".join("-" * widths[f] for f in fields) + " |"
    data_lines = [row_line(r) for r in results]
    out_md.write_text("\n".join([header, sep] + data_lines) + "\n", encoding="utf-8")


def write_summary(out_dir: Path, results: List[CheckResult]):
    summary: Dict[Tuple[str, str, str], List[CheckResult]] = {}
    for r in results:
        key = (r.variant, r.pie, r.mode)
        summary.setdefault(key, []).append(r)

    rows = []
    for key, lst in sorted(summary.items()):
        total = len(lst)
        passed = sum(1 for r in lst if r.status == "PASS")
        rows.append(
            {
                "variant": key[0],
                "pie": key[1],
                "mode": key[2],
                "pass_count": passed,
                "fail_count": total - passed,
                "total_checks": total,
                "pass_pct": f"{(passed / total * 100):.1f}" if total else "",
                "status": "PASS" if passed == total else "FAIL",
            }
        )

    fields = ["variant", "pie", "mode", "pass_count", "fail_count", "total_checks", "pass_pct", "status"]
    out_csv = out_dir / "fairness_summary.csv"
    out_md = out_dir / "fairness_summary.md"
    with out_csv.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)

    # Markdown
    widths = {f: len(f) for f in fields}
    for row in rows:
        for f in fields:
            widths[f] = max(widths[f], len(str(row[f])))

    def row_line(row: dict) -> str:
        return "| " + " | ".join(str(row[f]).ljust(widths[f]) for f in fields) + " |"

    header = "| " + " | ".join(f.ljust(widths[f]) for f in fields) + " |"
    sep = "| " + " | ".join("-" * widths[f] for f in fields) + " |"
    data_lines = [row_line(r) for r in rows]
    out_md.write_text("\n".join([header, sep] + data_lines) + "\n", encoding="utf-8")


def run_analyze_campaign(s1_joined_dir: Path, out_dir: Path, drop_warmup: int):
    cmd = [
        "python3",
        str(ANALYZE_SCRIPT),
        "--s1-joined-dir",
        str(s1_joined_dir),
        "--s2-dir",
        "data/csv",
        "--out-dir",
        str(out_dir),
        "--drop-warmup",
        str(drop_warmup),
    ]
    subprocess.run(cmd, check=True)


def run_qers(kpi_csv: Path, out_dir: Path, mode: str, w_sec: str):
    out_dir.mkdir(parents=True, exist_ok=True)
    cmd = [
        "python3",
        str(QERS_SCRIPT),
        "--kpi",
        str(kpi_csv),
        "--out-dir",
        str(out_dir),
        "--normalize-within-mode",
        "--mode",
        mode,
        "--w-sec",
        w_sec,
    ]
    subprocess.run(cmd, check=True)


def main():
    parser = argparse.ArgumentParser(description="Check S1 fairness (session/reconn) and optionally compute QERS.")
    parser.add_argument("--s1-joined-dir", default="data/csv_joined", help="Directory with joined S1 CSVs")
    parser.add_argument("--out-dir", default="data/analysis/fairness", help="Output directory")
    parser.add_argument("--drop-warmup", type=int, default=5, help="Drop iter < N")
    parser.add_argument("--modes", default="all", choices=["all", "session", "reconn"], help="Which modes to evaluate")
    parser.add_argument("--run-qers", action="store_true", help="Also compute QERS per-mode using KPI table")
    parser.add_argument("--w-sec", default="Pr=0.5,K=0.2,R=0.1,Co=0.2", help="Security weights override for compute_qers")

    # Thresholds (session)
    parser.add_argument("--rssi-target", type=float, default=-20.0)
    parser.add_argument("--rssi-tolerance", type=float, default=1.0)
    parser.add_argument("--net-cv-max", type=float, default=10.0)
    parser.add_argument("--net-ratio-max", type=float, default=1.20)
    parser.add_argument("--jitter-abs-max", type=float, default=10000.0)
    parser.add_argument("--jitter-ratio-max", type=float, default=0.4)
    parser.add_argument("--energy-cv-max", type=float, default=5.0)
    parser.add_argument("--energy-ratio-max", type=float, default=1.10)
    parser.add_argument("--cpu-cv-max", type=float, default=5.0)
    parser.add_argument("--keygen-cv-max", type=float, default=2.0)
    parser.add_argument("--decaps-cv-max", type=float, default=3.0)
    parser.add_argument("--encaps-cv-max", type=float, default=15.0)
    parser.add_argument("--encaps-ratio-max", type=float, default=1.30)

    # Thresholds (reconn)
    parser.add_argument("--reconnect-target-us", type=float, default=16_000_000.0)
    parser.add_argument("--reconnect-tol-pct", type=float, default=0.1)
    parser.add_argument("--reconnect-cv-max", type=float, default=0.5)
    parser.add_argument("--reconnect-ratio-max", type=float, default=1.003)
    parser.add_argument("--reconnect-energy-cv-max", type=float, default=0.5)

    args = parser.parse_args()

    s1_joined_dir = Path(args.s1_joined_dir)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # 1) Run analyze_campaign to regenerate stats/KPI
    run_analyze_campaign(s1_joined_dir, out_dir, args.drop_warmup)

    # 2) Load summaries
    stats = read_summary(out_dir / "s1_summary_stats.csv")
    qmap = read_quality(out_dir / "s1_quality_summary.csv")

    results: List[CheckResult] = []
    available_modes = set()
    for (variant, pie, mode), q in qmap.items():
        if args.modes != "all" and mode != args.modes:
            continue
        available_modes.add(mode)
        if mode == "session":
            evaluate_session(variant, pie, stats, q, args, results)
        elif mode == "reconn":
            evaluate_reconn(variant, pie, stats, q, args, results)

    # 3) Write reports
    write_checks(out_dir, results)
    write_summary(out_dir, results)

    # 4) Optional QERS
    if args.run_qers:
        kpi_csv = out_dir / "s1_kpi_table.csv"
        if "session" in available_modes:
            run_qers(kpi_csv, out_dir / "qers_session", "session", args.w_sec)
        if "reconn" in available_modes:
            run_qers(kpi_csv, out_dir / "qers_reconn", "reconn", args.w_sec)

    print(f"Fairness checks written to {out_dir}")


if __name__ == "__main__":
    main()
