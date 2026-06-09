#!/usr/bin/env python3
"""Compute QERS Basic and Tuned scores.

Derived from QERS paper notes (data/analysis/qers_paper_notes.md) and the
weighting approach used in tools/derive_weights.py (Entropy Weight Method +
Prior blending). Supports per-mode or global normalization.
"""
from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path
from typing import Iterable, Dict, List

MS_DEFAULT = 100.0


def parse_float(value: str | None) -> float | None:
    if value is None:
        return None
    s = value.strip()
    if s == "":
        return None
    try:
        return float(s)
    except ValueError:
        return None


def clamp(x: float, lo: float, hi: float) -> float:
    return lo if x < lo else hi if x > hi else x


def minmax_norm(x: float | None, xmin: float | None, xmax: float | None, ms: float = MS_DEFAULT) -> float:
    if x is None or xmin is None or xmax is None:
        return 0.0
    if xmax <= xmin:
        return 0.0
    v = ms * (x - xmin) / (xmax - xmin)
    return clamp(v, 0.0, ms)


def bounds(values: Iterable[float | None]) -> tuple[float | None, float | None]:
    vals = [v for v in values if v is not None and not (math.isnan(v) or math.isinf(v))]
    if not vals:
        return None, None
    return min(vals), max(vals)


def write_csv(path: Path, fieldnames: list[str], rows: list[dict[str, object]]):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def write_md_table(path: Path, fieldnames: list[str], rows: list[dict[str, object]]):
    def cell(row: dict[str, object], key: str) -> str:
        v = row.get(key, "")
        if v is None:
            return ""
        return str(v)

    widths = {h: len(h) for h in fieldnames}
    for row in rows:
        for h in fieldnames:
            widths[h] = max(widths[h], len(cell(row, h)))

    header_line = "| " + " | ".join(h.ljust(widths[h]) for h in fieldnames) + " |"
    sep_line = "| " + " | ".join("-" * widths[h] for h in fieldnames) + " |"
    data_lines = ["| " + " | ".join(cell(row, h).ljust(widths[h]) for h in fieldnames) + " |" for row in rows]

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join([header_line, sep_line] + data_lines) + "\n", encoding="utf-8")


def entropy_weights(matrix: List[Dict[str, float]], keys: List[str]) -> Dict[str, float]:
    n = len(matrix)
    if n == 0:
        return {k: 0.0 for k in keys}
    eps = 1e-12
    pij: Dict[str, List[float]] = {k: [] for k in keys}
    for k in keys:
        col = [max(0.0, min(1.0, row.get(k, 0.0))) for row in matrix]
        s = sum(col) + eps * n
        pij[k] = [(v + eps) / s for v in col]
    kconst = 1.0 / math.log(n)
    d: Dict[str, float] = {}
    for k in keys:
        e_k = -kconst * sum(p * math.log(p) for p in pij[k])
        d[k] = 1.0 - e_k
        if d[k] < 1e-6:
            d[k] = 0.0
    ssum = sum(d.values())
    if ssum <= 0:
        return {k: 1.0 / len(keys) for k in keys}
    return {k: d[k] / ssum for k in keys}


def renorm(weights: Dict[str, float]) -> Dict[str, float]:
    s = sum(max(0.0, v) for v in weights.values())
    if s <= 0:
        n = len(weights)
        return {k: 1.0 / n for k in weights}
    return {k: max(0.0, v) / s for k, v in weights.items()}


def parse_weights_arg(s: str | None, keys: List[str]) -> Dict[str, float] | None:
    if not s:
        return None
    out: Dict[str, float] = {k: 0.0 for k in keys}
    try:
        parts = [p for p in s.split(",") if p]
        for p in parts:
            k, v = p.split("=")
            k = k.strip()
            v = float(v.strip())
            if k in out:
                out[k] = v
    except Exception:
        return None
    ssum = sum(max(0.0, v) for v in out.values())
    if ssum > 0:
        out = {k: max(0.0, v) / ssum for k, v in out.items()}
    return out


def pr_level_from_variant(variant: str) -> float:
    if "512" in variant:
        return 1.0
    if "768" in variant:
        return 3.0
    if "1024" in variant:
        return 5.0
    return 0.0


def load_kpi_rows(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        return [dict(row) for row in reader]


def benefit_oriented(raw: dict[str, float | None], bnds: dict[str, tuple[float | None, float | None]], key: str,
                     orientation: str, ms: float) -> float:
    lo, hi = bnds.get(key, (None, None))
    if orientation == "benefit":
        return minmax_norm(raw.get(key), lo, hi, ms)
    # cost: higher is worse; keep the min-max value (no inversion)
    return minmax_norm(raw.get(key), lo, hi, ms)


def build_norms(rows: list[dict[str, str]], mode_bounds: dict[str, dict[str, tuple[float | None, float | None]]],
                global_bounds: dict[str, tuple[float | None, float | None]] | None, ms: float,
                mode: str, k_orientation: str, o_orientation: str) -> dict[str, float]:
    raw = {
        "L": parse_float(rows[0].get("total_latency_us_p50")),
        "J": parse_float(rows[0].get("jitter_us_p50")),
        "Ploss": parse_float(rows[0].get("packet_loss_rate")),
        "E": parse_float(rows[0].get("total_energy_uj_p50")),
        "C": parse_float(rows[0].get("cpu_util_percent_p50")),
        "R": parse_float(rows[0].get("rssi_dbm_p50")),
        "K": (parse_float(rows[0].get("pub_bytes_p50")) or 0.0) + (parse_float(rows[0].get("ct_bytes_p50")) or 0.0),
        "O": parse_float(rows[0].get("overhead_bytes_p50")),
        "Pr": pr_level_from_variant(rows[0].get("variant", "")),
    }
    b = global_bounds if global_bounds is not None else mode_bounds.get(mode, {})
    norms = {
        "L": benefit_oriented(raw, b, "L", "cost", ms),
        "O": benefit_oriented(raw, b, "O", "cost" if o_orientation == "cost" else "benefit", ms),
        "Ploss": benefit_oriented(raw, b, "Ploss", "cost", ms),
        "C": benefit_oriented(raw, b, "C", "cost", ms),
        "E": benefit_oriented(raw, b, "E", "cost", ms),
        "K": benefit_oriented(raw, b, "K", "cost" if k_orientation == "cost" else "benefit", ms),
        "R": benefit_oriented(raw, b, "R", "benefit", ms),
        "Pr": benefit_oriented(raw, b, "Pr", "benefit", ms),
    }
    return norms


def build_bounds(rows: list[dict[str, str]], by_mode: bool) -> tuple[dict[str, tuple[float | None, float | None]] | None,
                                                                      dict[str, dict[str, tuple[float | None, float | None]]]]:
    if by_mode:
        mode_keys = sorted({(r.get("mode") or "").strip() for r in rows})
        mode_bounds: dict[str, dict[str, tuple[float | None, float | None]]] = {}
        for mode in mode_keys:
            mode_rows = [r for r in rows if (r.get("mode") or "").strip() == mode]
            raw_list = []
            for r in mode_rows:
                raw_list.append({
                    "L": parse_float(r.get("total_latency_us_p50")),
                    "J": parse_float(r.get("jitter_us_p50")),
                    "Ploss": parse_float(r.get("packet_loss_rate")),
                    "E": parse_float(r.get("total_energy_uj_p50")),
                    "C": parse_float(r.get("cpu_util_percent_p50")),
                    "R": parse_float(r.get("rssi_dbm_p50")),
                    "K": (parse_float(r.get("pub_bytes_p50")) or 0.0) + (parse_float(r.get("ct_bytes_p50")) or 0.0),
                    "O": parse_float(r.get("overhead_bytes_p50")),
                    "Pr": pr_level_from_variant(r.get("variant", "")),
                })
            b: dict[str, tuple[float | None, float | None]] = {}
            for k in ("L", "J", "Ploss", "E", "C", "R", "K", "O", "Pr"):
                b[k] = bounds(raw[k] for raw in raw_list)
            mode_bounds[mode] = b
        # For per-mode normalization, we do not use global bounds. Return None to signal caller
        # to pick mode-specific bounds instead of an empty dict that would zero-out metrics.
        return None, mode_bounds
    else:
        raw_list = []
        for r in rows:
            raw_list.append({
                "L": parse_float(r.get("total_latency_us_p50")),
                "J": parse_float(r.get("jitter_us_p50")),
                "Ploss": parse_float(r.get("packet_loss_rate")),
                "E": parse_float(r.get("total_energy_uj_p50")),
                "C": parse_float(r.get("cpu_util_percent_p50")),
                "R": parse_float(r.get("rssi_dbm_p50")),
                "K": (parse_float(r.get("pub_bytes_p50")) or 0.0) + (parse_float(r.get("ct_bytes_p50")) or 0.0),
                "O": parse_float(r.get("overhead_bytes_p50")),
                "Pr": pr_level_from_variant(r.get("variant", "")),
            })
        global_bounds = {k: bounds(raw[k] for raw in raw_list) for k in ("L", "J", "Ploss", "E", "C", "R", "K", "O", "Pr")}
        return global_bounds, {}


def fmt(x: float | None, nd: int = 3) -> str:
    if x is None:
        return ""
    if math.isnan(x) or math.isinf(x):
        return ""
    return f"{x:.{nd}f}"


def compute_weights(norm_rows: List[Dict[str, float]], keys: List[str], prior: Dict[str, float], alpha: float) -> tuple[Dict[str, float], Dict[str, float], Dict[str, float]]:
    w_obj = entropy_weights(norm_rows, keys)
    prior_norm = renorm({k: prior.get(k, 0.0) for k in keys})
    w_final = renorm({k: alpha * w_obj.get(k, 0.0) + (1.0 - alpha) * prior_norm.get(k, 0.0) for k in keys})
    return w_obj, prior_norm, w_final


def main():
    parser = argparse.ArgumentParser(description="Compute QERS Basic/Tuned with EWM+Prior weighting")
    parser.add_argument("--kpi", default="data/analysis/s1_kpi_table.csv", help="Input KPI CSV")
    parser.add_argument("--out-dir", default="data/analysis/qers_basic_tuned", help="Output directory")
    parser.add_argument("--formula", choices=["basic", "tuned"], default="basic", help="Which QERS formula to compute")
    parser.add_argument("--mode", choices=["session", "reconn", "all"], default="all", help="Filter mode before normalization")
    parser.add_argument("--ms", type=float, default=MS_DEFAULT, help="Maximum score (default 100)")
    parser.add_argument("--normalize-within-mode", dest="norm_within", action="store_true", help="Normalize per-mode (default)")
    parser.add_argument("--normalize-global", dest="norm_within", action="store_false", help="Normalize globally (overrides per-mode)")
    parser.set_defaults(norm_within=True)
    parser.add_argument("--alpha-basic", type=float, default=0.5, help="Blend factor for Basic (obj vs prior)")
    parser.add_argument("--alpha-tuned", type=float, default=0.5, help="Blend factor for Tuned (obj vs prior)")
    parser.add_argument("--prior-basic", default="L=0.5,O=0.25,Ploss=0.25", help="Prior weights for Basic")
    parser.add_argument("--prior-tuned", default="L=0.25,O=0.15,Ploss=0.15,C=0.15,E=0.10,K=0.10,R=0.10", help="Prior weights for Tuned")
    parser.add_argument("--k-orientation", choices=["benefit", "cost"], default="cost", help="Orientation for K (default cost)")
    parser.add_argument("--o-orientation", choices=["benefit", "cost"], default="cost", help="Orientation for O/Co (default cost)")
    args = parser.parse_args()

    kpi_path = Path(args.kpi)
    out_dir = Path(args.out_dir)

    rows = load_kpi_rows(kpi_path)
    if args.mode != "all":
        mode_key = args.mode.strip()
        rows = [r for r in rows if (r.get("mode") or "").strip() == mode_key]
    if not rows:
        raise SystemExit("No rows found for given mode")

    global_bounds, mode_bounds = build_bounds(rows, by_mode=args.norm_within)

    keys_basic = ["L", "O", "Ploss"]
    keys_tuned = ["L", "O", "Ploss", "C", "E", "K", "R"]

    alpha_basic = clamp(args.alpha_basic, 0.0, 1.0)
    alpha_tuned = clamp(args.alpha_tuned, 0.0, 1.0)

    prior_basic = parse_weights_arg(args.prior_basic, keys_basic) or renorm({k: 1.0 for k in keys_basic})
    prior_tuned = parse_weights_arg(args.prior_tuned, keys_tuned) or renorm({k: 1.0 for k in keys_tuned})

    out_rows: list[dict[str, object]] = []
    norm_rows_for_weights: List[Dict[str, float]] = []

    for row in rows:
        mode = (row.get("mode") or "").strip()
        norms = build_norms([row], mode_bounds, global_bounds, 1.0, mode, args.k_orientation, args.o_orientation)  # 0..1 for EWM
        norm_rows_for_weights.append({k: norms[k] for k in (keys_basic if args.formula == "basic" else keys_tuned)})

    if args.formula == "basic":
        w_obj, w_prior, w_final = compute_weights(norm_rows_for_weights, keys_basic, prior_basic, alpha_basic)
    else:
        w_obj, w_prior, w_final = compute_weights(norm_rows_for_weights, keys_tuned, prior_tuned, alpha_tuned)

    # Compute scores with MS scaling (0..100) using same bounds
    out_rows = []
    for row in rows:
        variant = (row.get("variant") or "").strip()
        pie = (row.get("pie") or "").strip()
        mode = (row.get("mode") or "").strip()
        norms_100 = build_norms([row], mode_bounds, global_bounds, args.ms, mode, args.k_orientation, args.o_orientation)

        if args.formula == "basic":
            P = w_final["L"] * norms_100["L"] + w_final["O"] * norms_100["O"] + w_final["Ploss"] * norms_100["Ploss"]
            qers = clamp(args.ms - P, 0.0, args.ms)
        else:
            P = (
                w_final["L"] * norms_100["L"]
                + w_final["O"] * norms_100["O"]
                + w_final["Ploss"] * norms_100["Ploss"]
                + w_final["C"] * norms_100["C"]
                + w_final["E"] * norms_100["E"]
                + w_final["K"] * norms_100["K"]
            )
            qers = clamp(args.ms - P + w_final["R"] * norms_100["R"], 0.0, args.ms)

        out_rows.append({
            "variant": variant,
            "pie": pie,
            "mode": mode,
            "QERS": fmt(qers, 3),
            "P_component": fmt(P, 3),
            "L": fmt(norms_100["L"], 3),
            "O": fmt(norms_100["O"], 3),
            "Ploss": fmt(norms_100["Ploss"], 3),
            "C": fmt(norms_100["C"], 3),
            "E": fmt(norms_100["E"], 3),
            "K": fmt(norms_100["K"], 3),
            "R": fmt(norms_100["R"], 3),
            "Pr": fmt(norms_100["Pr"], 3),
        })

    out_rows_sorted = sorted(out_rows, key=lambda r: float(r.get("QERS") or 0.0), reverse=True)
    for i, r in enumerate(out_rows_sorted, start=1):
        r["rank_global"] = i

    # Write outputs
    out_dir.mkdir(parents=True, exist_ok=True)
    fname_prefix = "qers_basic" if args.formula == "basic" else "qers_tuned"
    fields = ["rank_global", "variant", "pie", "mode", "QERS", "P_component", "L", "O", "Ploss", "C", "E", "K", "R", "Pr"]
    write_csv(out_dir / f"{fname_prefix}_scores.csv", fields, out_rows_sorted)
    write_md_table(out_dir / f"{fname_prefix}_scores.md", fields, out_rows_sorted)

    # Weights summary
    weights_lines = []
    weights_lines.append(f"# QERS {args.formula.capitalize()} weights\n")
    weights_lines.append(f"- ms = {args.ms}\n")
    weights_lines.append(f"- normalize_within_mode = {args.norm_within}\n")
    weights_lines.append(f"- k_orientation = {args.k_orientation}, o_orientation = {args.o_orientation}\n\n")

    weights_lines.append("## Objective weights (EWM)\n")
    for k in (keys_basic if args.formula == "basic" else keys_tuned):
        weights_lines.append(f"- {k}: {fmt(w_obj[k], 4)}\n")

    weights_lines.append("\n## Prior weights (normalized)\n")
    for k in (keys_basic if args.formula == "basic" else keys_tuned):
        weights_lines.append(f"- {k}: {fmt(w_prior[k], 4)}\n")

    weights_lines.append("\n## Final weights (blended)\n")
    for k in (keys_basic if args.formula == "basic" else keys_tuned):
        weights_lines.append(f"- {k}: {fmt(w_final[k], 4)}\n")

    weights_lines.append("\n")
    weights_lines.append(f"alpha_basic = {fmt(alpha_basic,3)}\n")
    weights_lines.append(f"alpha_tuned = {fmt(alpha_tuned,3)}\n")

    (out_dir / f"{fname_prefix}_weights.md").write_text("".join(weights_lines), encoding="utf-8")

    print(f"Wrote QERS {args.formula} outputs to: {out_dir}")


if __name__ == "__main__":
    main()
