#!/usr/bin/env python3
import argparse
import csv
import json
from dataclasses import dataclass
from math import log
from pathlib import Path
from typing import Dict, List, Tuple


@dataclass
class WeightsResult:
    perf_obj: Dict[str, float]
    perf_prior: Dict[str, float]
    perf_final: Dict[str, float]
    sec_obj: Dict[str, float]
    sec_prior: Dict[str, float]
    sec_final: Dict[str, float]
    macro_dispersion: Dict[str, float]
    macro_weights_suggested: Dict[str, float]


PERF_KEYS = ["L", "E", "C", "J", "Ploss"]
SEC_KEYS = ["Pr", "K", "Co", "R"]
ALL_KEYS = PERF_KEYS + SEC_KEYS


def parse_float(x: str | None) -> float | None:
    if x is None:
        return None
    x = x.strip()
    if not x:
        return None
    try:
        return float(x)
    except ValueError:
        return None


def pr_level_from_variant(variant: str) -> float:
    v = (variant or "").strip().lower()
    if "1024" in v:
        return 5.0
    if "768" in v:
        return 3.0
    if "512" in v:
        return 1.0
    return 0.0


def load_kpi_rows(path: Path) -> List[Dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as fh:
        return [dict(r) for r in csv.DictReader(fh)]


def load_quality_rows(path: Path) -> Dict[Tuple[str, str, str], Dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as fh:
        out: Dict[Tuple[str, str, str], Dict[str, str]] = {}
        for r in csv.DictReader(fh):
            key = (r.get("variant", ""), r.get("pie", ""), r.get("mode", ""))
            out[key] = dict(r)
        return out


def bounds(values: List[float | None]) -> Tuple[float | None, float | None]:
    vals = [v for v in values if v is not None]
    if not vals:
        return None, None
    return min(vals), max(vals)


def minmax_to_benefit(x: float | None, lo: float | None, hi: float | None, orientation: str) -> float:
    if x is None or lo is None or hi is None or hi == lo:
        return 0.0
    # Normalize 0..1 using min-max, then flip if cost.
    norm = 0.0 if hi == lo else (x - lo) / (hi - lo)
    if orientation == "benefit":
        return max(0.0, min(1.0, norm))
    else:  # cost
        return max(0.0, min(1.0, 1.0 - norm))


def entropy_weights(matrix: List[Dict[str, float]], keys: List[str]) -> Dict[str, float]:
    # matrix: list of rows, each row has keys already benefit-oriented in [0,1]
    n = len(matrix)
    if n == 0:
        return {k: 0.0 for k in keys}
    # build pij for each key
    eps = 1e-12
    pij: Dict[str, List[float]] = {k: [] for k in keys}
    for k in keys:
        col = [max(0.0, min(1.0, row.get(k, 0.0))) for row in matrix]
        s = sum(col) + eps * n
        pij[k] = [(v + eps) / s for v in col]
    # entropy and diversification
    kconst = 1.0 / log(n)
    e: Dict[str, float] = {}
    d: Dict[str, float] = {}
    for k in keys:
        e[k] = -kconst * sum(p * log(p) for p in pij[k])
        d[k] = 1.0 - e[k]
    # zero-out near-constant criteria (very low diversification)
    for k in keys:
        if d[k] < 1e-6:
            d[k] = 0.0
    s = sum(d.values())
    if s <= 0:
        return {k: (1.0 / len(keys)) for k in keys}
    return {k: d[k] / s for k in keys}


def renorm(weights: Dict[str, float]) -> Dict[str, float]:
    s = sum(max(0.0, v) for v in weights.values())
    if s <= 0:
        n = len(weights)
        return {k: 1.0 / n for k in weights}
    return {k: max(0.0, v) / s for k, v in weights.items()}


def combine(obj: Dict[str, float], prior: Dict[str, float], alpha: float) -> Dict[str, float]:
    keys = obj.keys()
    out = {k: alpha * obj.get(k, 0.0) + (1.0 - alpha) * prior.get(k, 0.0) for k in keys}
    return renorm(out)


def macro_dispersion(rows: List[Dict[str, float]]) -> Dict[str, float]:
    # use average normalized range (0..1) across keys as dispersion proxy
    out: Dict[str, float] = {}
    for group, keys in {"P": PERF_KEYS, "S": SEC_KEYS}.items():
        vals_by_key = {k: [r[k] for r in rows] for k in keys}
        rngs: List[float] = []
        for k, col in vals_by_key.items():
            if not col:
                continue
            lo = min(col)
            hi = max(col)
            rngs.append(max(0.0, hi - lo))
        out[group] = sum(rngs) / len(rngs) if rngs else 0.0
    total = out.get("P", 0.0) + out.get("S", 0.0)
    out["P_weight_suggested"] = (out.get("P", 0.0) / total) if total > 0 else 0.5
    out["S_weight_suggested"] = 1.0 - out["P_weight_suggested"]
    return out


def build_rows(kpi_rows: List[Dict[str, str]], qual_map: Dict[Tuple[str, str, str], Dict[str, str]], mode_filter: str,
               k_orientation: str, co_orientation: str) -> List[Dict[str, float]]:
    # bounds per key within the filtered mode
    filtered = [r for r in kpi_rows if (r.get("mode", "").strip() == mode_filter or mode_filter == "all")]
    if not filtered:
        return []
    # gather raw metrics and bounds
    raw_list: List[Dict[str, float]] = []
    for r in filtered:
        key = (r.get("variant", ""), r.get("pie", ""), r.get("mode", ""))
        q = qual_map.get(key, {})
        total_latency = parse_float(r.get("total_latency_us_p50"))
        jitter = parse_float(r.get("jitter_us_p50"))
        total_energy = parse_float(r.get("total_energy_uj_p50"))
        cpu_util = parse_float(r.get("cpu_util_percent_p50"))
        rssi = parse_float(r.get("rssi_dbm_p50"))
        pub_b = parse_float(r.get("pub_bytes_p50"))
        ct_b = parse_float(r.get("ct_bytes_p50"))
        overhead_b = parse_float(r.get("overhead_bytes_p50"))
        ploss = parse_float(q.get("packet_loss_rate"))
        k_bytes = (pub_b + ct_b) if pub_b is not None and ct_b is not None else None
        pr_level = pr_level_from_variant(r.get("variant", ""))
        raw_list.append({
            "L": total_latency or 0.0,
            "E": total_energy or 0.0,
            "C": cpu_util or 0.0,
            "J": jitter or 0.0,
            "Ploss": ploss or 0.0,
            "R": rssi or 0.0,
            "K": k_bytes or 0.0,
            "Co": overhead_b or 0.0,
            "Pr": pr_level,
        })
    # compute bounds for min-max
    bnds: Dict[str, Tuple[float | None, float | None]] = {}
    for k in ALL_KEYS:
        lo, hi = bounds([row[k] for row in raw_list])
        bnds[k] = (lo, hi)
    # convert to benefit-oriented 0..1 for entropy & macro dispersion
    out_rows: List[Dict[str, float]] = []
    for row in raw_list:
        bn: Dict[str, float] = {}
        bn["L"] = minmax_to_benefit(row["L"], *bnds["L"], orientation="cost")
        bn["E"] = minmax_to_benefit(row["E"], *bnds["E"], orientation="cost")
        bn["C"] = minmax_to_benefit(row["C"], *bnds["C"], orientation="cost")
        bn["J"] = minmax_to_benefit(row["J"], *bnds["J"], orientation="cost")
        bn["Ploss"] = minmax_to_benefit(row["Ploss"], *bnds["Ploss"], orientation="cost")
        bn["Pr"] = minmax_to_benefit(row["Pr"], *bnds["Pr"], orientation="benefit")
        bn["K"] = minmax_to_benefit(row["K"], *bnds["K"], orientation=("benefit" if k_orientation == "benefit" else "cost"))
        bn["Co"] = minmax_to_benefit(row["Co"], *bnds["Co"], orientation=("benefit" if co_orientation == "benefit" else "cost"))
        bn["R"] = minmax_to_benefit(row["R"], *bnds["R"], orientation="benefit")
        out_rows.append(bn)
    return out_rows


def main():
    p = argparse.ArgumentParser(description="Derive AHP weights using Entropy + Prior hybrid")
    p.add_argument("--kpi", default="data/analysis/s1_kpi_table.csv")
    p.add_argument("--quality", default="data/analysis/s1_quality_summary.csv")
    p.add_argument("--mode", default="session", choices=["session", "reconn", "all"], help="Decision set scope")
    p.add_argument("--alpha", type=float, default=0.5, help="Blend factor for Entropy vs Prior (0..1)")
    p.add_argument("--k-orientation", choices=["cost", "benefit"], default="cost")
    p.add_argument("--co-orientation", choices=["cost", "benefit"], default="cost")
    p.add_argument("--out", default="data/analysis/weights")
    args = p.parse_args()

    kpi_path = Path(args.kpi)
    qual_path = Path(args.quality)
    out_dir = Path(args.out)

    kpi_rows = load_kpi_rows(kpi_path)
    qual_map = load_quality_rows(qual_path)

    rows = build_rows(kpi_rows, qual_map, args.mode, args.k_orientation, args.co_orientation)
    if not rows:
        raise SystemExit("No rows found for mode")

    # Objective entropy weights
    perf_obj = entropy_weights(rows, PERF_KEYS)
    sec_obj = entropy_weights(rows, SEC_KEYS)

    # Priors (policy/swing) — can be edited to reflect domain priorities
    perf_prior = {"L": 0.3, "E": 0.3, "C": 0.3, "J": 0.05, "Ploss": 0.05}
    sec_prior = {"Pr": 0.40, "K": 0.25, "Co": 0.3, "R": 0.05}

    # Hybrid final
    perf_final = combine(perf_obj, perf_prior, args.alpha)
    sec_final = combine(sec_obj, sec_prior, args.alpha)

    # Macro dispersion and suggested P,S split
    md = macro_dispersion(rows)
    macro_suggested = {"P": md.get("P_weight_suggested", 0.6), "S": md.get("S_weight_suggested", 0.4)}

    out_dir.mkdir(parents=True, exist_ok=True)

    # JSON
    res = WeightsResult(
        perf_obj=perf_obj,
        perf_prior=perf_prior,
        perf_final=perf_final,
        sec_obj=sec_obj,
        sec_prior=sec_prior,
        sec_final=sec_final,
        macro_dispersion=md,
        macro_weights_suggested=macro_suggested,
    )
    (out_dir / "weights_summary.json").write_text(json.dumps(res.__dict__, indent=2), encoding="utf-8")

    # Markdown summary
    def fmt(d: Dict[str, float]) -> str:
        return "\n".join(f"- {k}: {v:.3f}" for k, v in d.items())

    md_lines = [
        "# Derived Weights (Entropy × Prior)",
        "\n## Macro weights suggested (dispersion-based)",
        f"- P: {macro_suggested['P']:.3f}",
        f"- S: {macro_suggested['S']:.3f}",
        "\n## Performance objective (Entropy)",
        fmt(perf_obj),
        "\n## Performance final (Hybrid)",
        fmt(perf_final),
        "\n## Security objective (Entropy)",
        fmt(sec_obj),
        "\n## Security final (Hybrid)",
        fmt(sec_final),
        "\nNotes: objective weights are computed per-mode; priors reflect domain assumptions and can be edited.",
    ]
    (out_dir / "weights_summary.md").write_text("\n".join(md_lines) + "\n", encoding="utf-8")

    print(f"Wrote {out_dir / 'weights_summary.json'} and {out_dir / 'weights_summary.md'}")


if __name__ == "__main__":
    main()
