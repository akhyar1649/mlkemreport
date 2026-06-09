#!/usr/bin/env python3
"""Analyze the full S1/S2 experiment campaign.

Goals:
- Load all Scenario 2 CSVs from `data/csv/`.
- Load all Scenario 1 *joined* CSVs from `data/csv_joined/`.
- Compute descriptive stats (n, mean, sd, CV, p50, p95, min, max).
- Produce derived totals for S1 reconnect mode:
  - total_latency_us = handshake_us + reconnect_us
  - total_energy_uj  = energy_uj + reconnect_energy_uj
- Produce campaign summary tables:
  - S2 metric summary
  - S1 metric summary + quality summary (packet_loss_rate, ss_valid_rate)
  - PIE speedup tables (S2 and S1)
  - Reconnect overhead tables (S1)

No third-party dependencies.
"""

from __future__ import annotations

import argparse
import csv
import math
import re
from dataclasses import dataclass
from pathlib import Path
from statistics import mean, stdev


S2_RE = re.compile(r"^s2_(512|768|1024)_(pieon|pieoff)_(\d{8}_\d{6})\.csv$")
S1_JOINED_RE = re.compile(
    r"^s1_(512|768|1024)_(pieon|pieoff)_(reconn|session)_(\d{8}_\d{6})(?:_joined)?\.csv$"
)


def percentile(values: list[float], p: float) -> float | None:
    if not values:
        return None
    values = sorted(values)
    k = (len(values) - 1) * p
    f = math.floor(k)
    c = math.ceil(k)
    if f == c:
        return values[int(k)]
    return values[f] + (values[c] - values[f]) * (k - f)


@dataclass(frozen=True)
class Summary:
    n: int
    mean: float
    sd: float
    cv_pct: float | None
    p50: float
    p95: float
    min: float
    max: float


def summarize(values: list[float]) -> Summary | None:
    if not values:
        return None
    values_sorted = sorted(values)
    n = len(values_sorted)
    m = mean(values_sorted)
    sd = stdev(values_sorted) if n >= 2 else 0.0
    cv_pct = (sd / m * 100.0) if m != 0 else None
    p50 = percentile(values_sorted, 0.50)
    p95 = percentile(values_sorted, 0.95)
    return Summary(
        n=n,
        mean=m,
        sd=sd,
        cv_pct=cv_pct,
        p50=p50 if p50 is not None else float("nan"),
        p95=p95 if p95 is not None else float("nan"),
        min=values_sorted[0],
        max=values_sorted[-1],
    )


def fmt_num(value: float | None, decimals: int = 2) -> str:
    if value is None:
        return ""
    if isinstance(value, float) and (math.isnan(value) or math.isinf(value)):
        return ""
    if abs(value - round(value)) < 1e-9:
        return str(int(round(value)))
    return f"{value:.{decimals}f}"


def write_csv(path: Path, fieldnames: list[str], rows: list[dict[str, object]]):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def write_md_table(path: Path, fieldnames: list[str], rows: list[dict[str, object]]):
    def cell(row, key) -> str:
        value = row.get(key, "")
        if value is None:
            return ""
        return str(value)

    widths = {h: len(h) for h in fieldnames}
    for row in rows:
        for h in fieldnames:
            widths[h] = max(widths[h], len(cell(row, h)))

    header_line = "| " + " | ".join(h.ljust(widths[h]) for h in fieldnames) + " |"
    sep_line = "| " + " | ".join("-" * widths[h] for h in fieldnames) + " |"
    data_lines = [
        "| " + " | ".join(cell(row, h).ljust(widths[h]) for h in fieldnames) + " |" for row in rows
    ]

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join([header_line, sep_line] + data_lines) + "\n", encoding="utf-8")


def iter_s2_files(folder: Path) -> list[tuple[str, str, Path]]:
    results: list[tuple[str, str, Path]] = []
    for path in sorted(folder.glob("s2_*.csv")):
        match = S2_RE.match(path.name)
        if not match:
            continue
        variant_tag, pie_tag, _ts = match.groups()
        results.append((variant_tag, pie_tag, path))
    return results


def iter_s1_joined_files(folder: Path) -> list[tuple[str, str, str, Path]]:
    results: list[tuple[str, str, str, Path]] = []
    for path in sorted(folder.glob("s1_*.csv")):
        match = S1_JOINED_RE.match(path.name)
        if not match:
            continue
        variant_tag, pie_tag, mode_tag, _ts = match.groups()
        results.append((variant_tag, pie_tag, mode_tag, path))
    return results


def parse_int(value: str | None) -> int | None:
    if value is None:
        return None
    value = value.strip()
    if value == "":
        return None
    try:
        return int(value)
    except ValueError:
        return None


def parse_float(value: str | None) -> float | None:
    if value is None:
        return None
    value = value.strip()
    if value == "":
        return None
    try:
        return float(value)
    except ValueError:
        return None


def collect_s2(s2_dir: Path, drop_warmup: int):
    # key: (variant, pie, op) -> metric lists
    buckets: dict[tuple[str, str, str], dict[str, list[float]]] = {}
    for _variant_tag, pie_tag, path in iter_s2_files(s2_dir):
        with path.open("r", encoding="utf-8", newline="") as handle:
            reader = csv.DictReader(handle)
            for row in reader:
                if (row.get("scenario") or "").strip() != "s2":
                    continue
                iter_idx = parse_int(row.get("iter"))
                if iter_idx is None or iter_idx < drop_warmup:
                    continue
                variant = (row.get("variant") or "").strip() or "unknown"
                op = (row.get("op") or "").strip() or "unknown"
                key = (variant, pie_tag, op)
                bucket = buckets.setdefault(key, {"cycles": [], "time_us": [], "energy_uj": []})

                for col in ("cycles", "time_us", "energy_uj"):
                    value = parse_float(row.get(col))
                    if value is None or value < 0:
                        continue
                    bucket[col].append(value)
    return buckets


def collect_s1_joined(s1_joined_dir: Path, drop_warmup: int, success_only: bool):
    # key: (variant, pie, mode) -> metric lists
    metric_cols = [
        "keygen_us",
        "decaps_us",
        "handshake_us",
        "net_latency_us",
        "jitter_us",
        "energy_uj",
        "reconnect_us",
        "reconnect_energy_uj",
        "total_latency_us",
        "total_energy_uj",
        "cpu_util_percent",
        "rssi_dbm",
        "encaps_us",
        "pub_bytes",
        "ct_bytes",
        "overhead_bytes",
        "crypto_overhead_bytes",
    ]

    buckets: dict[tuple[str, str, str], dict[str, list[float]]] = {}
    quality: dict[tuple[str, str, str], dict[str, int]] = {}

    for _variant_tag, pie_tag, mode_tag, path in iter_s1_joined_files(s1_joined_dir):
        with path.open("r", encoding="utf-8", newline="") as handle:
            reader = csv.DictReader(handle)
            for row in reader:
                if (row.get("scenario") or "").strip() != "s1":
                    continue
                iter_idx = parse_int(row.get("iter"))
                if iter_idx is None or iter_idx < drop_warmup:
                    continue

                variant = (row.get("variant") or "").strip() or "unknown"
                key = (variant, pie_tag, mode_tag)

                packet_loss = parse_int(row.get("packet_loss")) or 0
                q = quality.setdefault(
                    key,
                    {
                        "n_total": 0,
                        "n_success": 0,
                        "loss_count": 0,
                        "ss_valid_total": 0,
                        "ss_valid_success": 0,
                    },
                )
                q["n_total"] += 1
                q["loss_count"] += 1 if packet_loss else 0

                ss_valid = parse_int(row.get("ss_valid"))
                if ss_valid is not None:
                    q["ss_valid_total"] += 1 if ss_valid else 0

                if packet_loss and success_only:
                    continue

                if not packet_loss:
                    q["n_success"] += 1
                    if ss_valid is not None:
                        q["ss_valid_success"] += 1 if ss_valid else 0

                bucket = buckets.setdefault(key, {name: [] for name in metric_cols})

                def add(col: str, value: float | None, *, skip_negative: bool = True):
                    if value is None:
                        return
                    if skip_negative and value < 0:
                        return
                    bucket[col].append(value)

                keygen_us = parse_float(row.get("keygen_us"))
                decaps_us = parse_float(row.get("decaps_us"))
                handshake_us = parse_float(row.get("handshake_us"))
                net_latency_us = parse_float(row.get("net_latency_us"))
                jitter_us = parse_float(row.get("jitter_us"))
                energy_uj = parse_float(row.get("energy_uj"))
                reconnect_us = parse_float(row.get("reconnect_us"))
                reconnect_energy_uj = parse_float(row.get("reconnect_energy_uj"))
                cpu_util = parse_float(row.get("cpu_util_percent"))
                rssi_dbm = parse_float(row.get("rssi_dbm"))
                encaps_us = parse_float(row.get("encaps_us"))

                add("keygen_us", keygen_us)
                add("decaps_us", decaps_us)
                add("handshake_us", handshake_us)
                add("net_latency_us", net_latency_us)
                add("jitter_us", jitter_us)
                add("energy_uj", energy_uj)
                add("reconnect_us", reconnect_us)
                add("reconnect_energy_uj", reconnect_energy_uj)

                if handshake_us is not None and reconnect_us is not None:
                    add("total_latency_us", handshake_us + reconnect_us)
                if energy_uj is not None and reconnect_energy_uj is not None:
                    add("total_energy_uj", energy_uj + reconnect_energy_uj)

                add("cpu_util_percent", cpu_util, skip_negative=False)
                add("rssi_dbm", rssi_dbm, skip_negative=False)
                add("encaps_us", encaps_us)

                pub_bytes = parse_float(row.get("pub_bytes"))
                ct_bytes = parse_float(row.get("ct_bytes"))
                overhead_bytes = parse_float(row.get("overhead_bytes"))
                add("pub_bytes", pub_bytes)
                add("ct_bytes", ct_bytes)
                add("overhead_bytes", overhead_bytes)
                if pub_bytes is not None and ct_bytes is not None and overhead_bytes is not None:
                    add("crypto_overhead_bytes", pub_bytes + ct_bytes + overhead_bytes)

    return buckets, quality


def build_metric_rows_s2(buckets):
    rows: list[dict[str, object]] = []
    for (variant, pie, op), metrics in sorted(buckets.items()):
        for metric_name, values in metrics.items():
            summary = summarize(values)
            if summary is None:
                continue
            rows.append(
                {
                    "scenario": "s2",
                    "variant": variant,
                    "pie": pie,
                    "op": op,
                    "metric": metric_name,
                    "n": summary.n,
                    "mean": fmt_num(summary.mean, 2),
                    "sd": fmt_num(summary.sd, 2),
                    "cv_pct": fmt_num(summary.cv_pct, 2) if summary.cv_pct is not None else "",
                    "p50": fmt_num(summary.p50, 2),
                    "p95": fmt_num(summary.p95, 2),
                    "min": fmt_num(summary.min, 2),
                    "max": fmt_num(summary.max, 2),
                }
            )
    return rows


def build_metric_rows_s1(buckets):
    rows: list[dict[str, object]] = []
    for (variant, pie, mode), metrics in sorted(buckets.items()):
        for metric_name, values in metrics.items():
            summary = summarize(values)
            if summary is None:
                continue
            rows.append(
                {
                    "scenario": "s1",
                    "variant": variant,
                    "pie": pie,
                    "mode": mode,
                    "metric": metric_name,
                    "n": summary.n,
                    "mean": fmt_num(summary.mean, 2),
                    "sd": fmt_num(summary.sd, 2),
                    "cv_pct": fmt_num(summary.cv_pct, 2) if summary.cv_pct is not None else "",
                    "p50": fmt_num(summary.p50, 2),
                    "p95": fmt_num(summary.p95, 2),
                    "min": fmt_num(summary.min, 2),
                    "max": fmt_num(summary.max, 2),
                }
            )
    return rows


def build_quality_rows_s1(quality):
    rows: list[dict[str, object]] = []
    for (variant, pie, mode), q in sorted(quality.items()):
        n_total = q.get("n_total", 0)
        n_success = q.get("n_success", 0)
        loss_count = q.get("loss_count", 0)
        ss_valid_total = q.get("ss_valid_total", 0)
        ss_valid_success = q.get("ss_valid_success", 0)

        packet_loss_rate = (loss_count / n_total) if n_total else None
        ss_valid_rate_all = (ss_valid_total / n_total) if n_total else None
        ss_valid_rate_success = (ss_valid_success / n_success) if n_success else None

        rows.append(
            {
                "variant": variant,
                "pie": pie,
                "mode": mode,
                "n_total": n_total,
                "n_success": n_success,
                "packet_loss_rate": fmt_num(packet_loss_rate, 4) if packet_loss_rate is not None else "",
                "ss_valid_rate_all": fmt_num(ss_valid_rate_all, 4) if ss_valid_rate_all is not None else "",
                "ss_valid_rate_success": fmt_num(ss_valid_rate_success, 4)
                if ss_valid_rate_success is not None
                else "",
            }
        )
    return rows


def index_p50(rows, key_fields: list[str]):
    index: dict[tuple[str, ...], float] = {}
    for row in rows:
        try:
            p50 = float(row.get("p50") or "")
        except ValueError:
            continue
        key = tuple(str(row.get(f) or "") for f in key_fields)
        index[key] = p50
    return index


def build_speedup_rows_s2(s2_rows):
    # compare pieoff vs pieon for each (variant, op, metric)
    idx = index_p50(s2_rows, ["variant", "pie", "op", "metric"])
    rows: list[dict[str, object]] = []

    keys = set((v, op, metric) for (v, pie, op, metric) in idx)
    for variant, op, metric in sorted(keys):
        off = idx.get((variant, "pieoff", op, metric))
        on = idx.get((variant, "pieon", op, metric))
        if off is None or on is None or off == 0:
            continue
        speedup = off / on if on else None
        delta_pct = ((on - off) / off * 100.0) if off else None
        rows.append(
            {
                "variant": variant,
                "op": op,
                "metric": metric,
                "pieoff_p50": fmt_num(off, 2),
                "pieon_p50": fmt_num(on, 2),
                "speedup": fmt_num(speedup, 3) if speedup is not None else "",
                "delta_pct": fmt_num(delta_pct, 2) if delta_pct is not None else "",
            }
        )
    return rows


def build_speedup_rows_s1(s1_rows, metric_allowlist: set[str] | None):
    # compare pieoff vs pieon for each (variant, mode, metric)
    idx = index_p50(s1_rows, ["variant", "pie", "mode", "metric"])
    rows: list[dict[str, object]] = []

    keys = set((v, mode, metric) for (v, pie, mode, metric) in idx)
    for variant, mode, metric in sorted(keys):
        if metric_allowlist is not None and metric not in metric_allowlist:
            continue
        off = idx.get((variant, "pieoff", mode, metric))
        on = idx.get((variant, "pieon", mode, metric))
        if off is None or on is None or off == 0:
            continue
        speedup = off / on if on else None
        delta_pct = ((on - off) / off * 100.0) if off else None
        rows.append(
            {
                "variant": variant,
                "mode": mode,
                "metric": metric,
                "pieoff_p50": fmt_num(off, 2),
                "pieon_p50": fmt_num(on, 2),
                "speedup": fmt_num(speedup, 3) if speedup is not None else "",
                "delta_pct": fmt_num(delta_pct, 2) if delta_pct is not None else "",
            }
        )
    return rows


def build_reconnect_rows_s1(s1_rows, metric_allowlist: set[str] | None):
    # compare session vs reconn for each (variant, pie, metric)
    idx = index_p50(s1_rows, ["variant", "pie", "mode", "metric"])
    rows: list[dict[str, object]] = []

    keys = set((v, pie, metric) for (v, pie, mode, metric) in idx)
    for variant, pie, metric in sorted(keys):
        if metric_allowlist is not None and metric not in metric_allowlist:
            continue
        session = idx.get((variant, pie, "session", metric))
        reconn = idx.get((variant, pie, "reconn", metric))
        if session is None or reconn is None:
            continue
        overhead = reconn - session
        overhead_pct = (overhead / session * 100.0) if session else None
        rows.append(
            {
                "variant": variant,
                "pie": pie,
                "metric": metric,
                "session_p50": fmt_num(session, 2),
                "reconn_p50": fmt_num(reconn, 2),
                "overhead": fmt_num(overhead, 2),
                "overhead_pct": fmt_num(overhead_pct, 2) if overhead_pct is not None else "",
            }
        )
    return rows


def build_kpi_rows_s1(s1_rows, s1_quality_rows):
    """Build a wide KPI table for thesis/reporting.

    Each row is one configuration (variant × pie × mode) and contains p50/p95
    for selected metrics, plus packet loss and ss_valid rates.
    """

    kpi_metrics = [
        # compute
        "keygen_us",
        "decaps_us",
        # subscriber compute (host)
        "encaps_us",
        # end-to-end
        "handshake_us",
        "net_latency_us",
        "jitter_us",
        # energy
        "energy_uj",
        "rssi_dbm",
        # reconnect window + derived totals
        "reconnect_us",
        "reconnect_energy_uj",
        "total_latency_us",
        "total_energy_uj",
        # resource-ish
        "cpu_util_percent",
        "crypto_overhead_bytes",
        "pub_bytes",
        "ct_bytes",
        "overhead_bytes",
    ]

    pivot: dict[tuple[str, str, str], dict[str, object]] = {}
    for row in s1_rows:
        variant = str(row.get("variant") or "")
        pie = str(row.get("pie") or "")
        mode = str(row.get("mode") or "")
        metric = str(row.get("metric") or "")
        if metric not in kpi_metrics:
            continue

        key = (variant, pie, mode)
        out = pivot.setdefault(
            key,
            {
                "variant": variant,
                "pie": pie,
                "mode": mode,
            },
        )
        out[f"{metric}_p50"] = row.get("p50", "")
        out[f"{metric}_p95"] = row.get("p95", "")

    quality_index = {
        (str(r.get("variant") or ""), str(r.get("pie") or ""), str(r.get("mode") or "")): r
        for r in s1_quality_rows
    }
    for key, out in pivot.items():
        q = quality_index.get(key)
        if q:
            out["packet_loss_rate"] = q.get("packet_loss_rate", "")
            out["ss_valid_rate_success"] = q.get("ss_valid_rate_success", "")

    fieldnames = ["variant", "pie", "mode"]
    for m in kpi_metrics:
        fieldnames.append(f"{m}_p50")
        fieldnames.append(f"{m}_p95")
    fieldnames += ["packet_loss_rate", "ss_valid_rate_success"]

    rows = [pivot[key] for key in sorted(pivot.keys())]
    return fieldnames, rows


def main():
    parser = argparse.ArgumentParser(description="Analyze S1/S2 campaign CSVs and emit summary tables.")
    parser.add_argument("--s2-dir", default="data/csv", help="Directory with S2 CSVs")
    parser.add_argument("--s1-joined-dir", default="data/csv_joined", help="Directory with joined S1 CSVs")
    parser.add_argument("--out-dir", default="data/analysis", help="Output directory")
    parser.add_argument("--drop-warmup", type=int, default=5, help="Drop iter < N")
    parser.add_argument(
        "--include-loss-samples",
        action="store_true",
        help="Include packet_loss=1 rows in metric distributions (not recommended for RTT/energy).",
    )
    args = parser.parse_args()

    s2_dir = Path(args.s2_dir)
    s1_joined_dir = Path(args.s1_joined_dir)
    out_dir = Path(args.out_dir)

    success_only = not args.include_loss_samples

    s2_buckets = collect_s2(s2_dir, args.drop_warmup)
    s1_buckets, s1_quality = collect_s1_joined(s1_joined_dir, args.drop_warmup, success_only=success_only)

    s2_rows = build_metric_rows_s2(s2_buckets)
    s1_rows = build_metric_rows_s1(s1_buckets)
    s1_quality_rows = build_quality_rows_s1(s1_quality)

    s2_fields = [
        "scenario",
        "variant",
        "pie",
        "op",
        "metric",
        "n",
        "mean",
        "sd",
        "cv_pct",
        "p50",
        "p95",
        "min",
        "max",
    ]
    s1_fields = [
        "scenario",
        "variant",
        "pie",
        "mode",
        "metric",
        "n",
        "mean",
        "sd",
        "cv_pct",
        "p50",
        "p95",
        "min",
        "max",
    ]
    s1_quality_fields = [
        "variant",
        "pie",
        "mode",
        "n_total",
        "n_success",
        "packet_loss_rate",
        "ss_valid_rate_all",
        "ss_valid_rate_success",
    ]

    write_csv(out_dir / "s2_summary_stats.csv", s2_fields, s2_rows)
    write_csv(out_dir / "s1_summary_stats.csv", s1_fields, s1_rows)
    write_csv(out_dir / "s1_quality_summary.csv", s1_quality_fields, s1_quality_rows)

    write_md_table(out_dir / "s2_summary_stats.md", s2_fields, s2_rows)
    write_md_table(out_dir / "s1_summary_stats.md", s1_fields, s1_rows)
    write_md_table(out_dir / "s1_quality_summary.md", s1_quality_fields, s1_quality_rows)

    s2_speedup_rows = build_speedup_rows_s2(s2_rows)
    s2_speedup_fields = ["variant", "op", "metric", "pieoff_p50", "pieon_p50", "speedup", "delta_pct"]
    write_csv(out_dir / "pie_speedup_s2.csv", s2_speedup_fields, s2_speedup_rows)
    write_md_table(out_dir / "pie_speedup_s2.md", s2_speedup_fields, s2_speedup_rows)

    s1_speedup_metrics = {
        "keygen_us",
        "decaps_us",
        "handshake_us",
        "energy_uj",
        "total_latency_us",
        "total_energy_uj",
        "cpu_util_percent",
    }
    s1_speedup_rows = build_speedup_rows_s1(s1_rows, metric_allowlist=s1_speedup_metrics)
    s1_speedup_fields = ["variant", "mode", "metric", "pieoff_p50", "pieon_p50", "speedup", "delta_pct"]
    write_csv(out_dir / "pie_speedup_s1.csv", s1_speedup_fields, s1_speedup_rows)
    write_md_table(out_dir / "pie_speedup_s1.md", s1_speedup_fields, s1_speedup_rows)

    reconn_metrics = {
        "reconnect_us",
        "reconnect_energy_uj",
        "total_latency_us",
        "total_energy_uj",
        "handshake_us",
        "energy_uj",
    }
    reconn_rows = build_reconnect_rows_s1(s1_rows, metric_allowlist=reconn_metrics)
    reconn_fields = ["variant", "pie", "metric", "session_p50", "reconn_p50", "overhead", "overhead_pct"]
    write_csv(out_dir / "reconnect_comparison.csv", reconn_fields, reconn_rows)
    write_md_table(out_dir / "reconnect_comparison.md", reconn_fields, reconn_rows)

    s1_kpi_fields, s1_kpi_rows = build_kpi_rows_s1(s1_rows, s1_quality_rows)
    write_csv(out_dir / "s1_kpi_table.csv", s1_kpi_fields, s1_kpi_rows)
    write_md_table(out_dir / "s1_kpi_table.md", s1_kpi_fields, s1_kpi_rows)

    print(f"Wrote summaries to: {out_dir}")


if __name__ == "__main__":
    main()
