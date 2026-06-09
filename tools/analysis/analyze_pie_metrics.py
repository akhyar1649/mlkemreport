#!/usr/bin/env python3
import argparse
import csv
import math
from pathlib import Path


def percentile(values, p):
    if not values:
        return None
    values = sorted(values)
    k = (len(values) - 1) * p
    f = math.floor(k)
    c = math.ceil(k)
    if f == c:
        return values[int(k)]
    return values[f] + (values[c] - values[f]) * (k - f)


def stats(values):
    return {
        "p50": percentile(values, 0.50),
        "p95": percentile(values, 0.95),
    }


def fmt_num(value, decimals=2):
    if value is None:
        return "n/a"
    if abs(value - round(value)) < 1e-9:
        return str(int(round(value)))
    return f"{value:.{decimals}f}"


def fmt_p50_p95(stat):
    if stat["p50"] is None and stat["p95"] is None:
        return "n/a"
    if stat["p95"] is None:
        return fmt_num(stat["p50"])
    return f"{fmt_num(stat['p50'])}/{fmt_num(stat['p95'])}"


def parse_s2(path, drop_warmup):
    data = {}
    variant = None
    with open(path, "r", newline="") as handle:
        reader = csv.reader(handle)
        for row in reader:
            if not row or row[0] != "s2":
                continue
            try:
                iter_idx = int(row[2])
            except ValueError:
                continue
            if iter_idx < drop_warmup:
                continue
            variant = variant or row[1]
            op = row[3]
            cycles = float(row[4])
            time_us = float(row[5])
            energy_uj = float(row[6])
            data.setdefault(op, {"cycles": [], "time_us": [], "energy_uj": []})
            data[op]["cycles"].append(cycles)
            data[op]["time_us"].append(time_us)
            data[op]["energy_uj"].append(energy_uj)
    return variant or "unknown", data


def parse_s1(path, drop_warmup, ignore_loss):
    metrics = {
        "keygen_us": [],
        "decaps_us": [],
        "handshake_us": [],
        "net_latency_us": [],
        "jitter_us": [],
        "energy_uj": [],
        "reconnect_us": [],
        "reconnect_energy_uj": [],
        "cpu_util_percent": [],
    }

    variant = None
    loss_count = 0
    total = 0
    header = None
    idx = {}

    def index(name, fallback=None):
        if name in idx:
            return idx[name]
        return fallback

    with open(path, "r", newline="") as handle:
        reader = csv.reader(handle)
        for row in reader:
            if not row:
                continue

            first = (row[0] or "").strip().lower()
            if first == "scenario":
                header = [c.strip() for c in row]
                idx = {name: i for i, name in enumerate(header) if name}
                continue

            if first != "s1":
                continue

            iter_col = index("iter", 2)
            try:
                iter_idx = int(row[iter_col])
            except (ValueError, TypeError, IndexError):
                continue
            if iter_idx < drop_warmup:
                continue

            variant_col = index("variant", 1)
            try:
                variant = variant or row[variant_col]
            except IndexError:
                variant = variant or "unknown"

            total += 1
            packet_loss_col = index("packet_loss", 14)
            try:
                packet_loss = int(row[packet_loss_col])
            except (ValueError, TypeError, IndexError):
                packet_loss = 0
            loss_count += packet_loss

            # By default, exclude loss samples from metric distributions.
            if ignore_loss and packet_loss == 1:
                continue

            def add_metric(metric_name, col_name, fallback_index=None, skip_negative=True):
                col = index(col_name, fallback_index)
                if col is None:
                    return
                try:
                    value = float(row[col])
                except (ValueError, TypeError, IndexError):
                    return
                if skip_negative and value < 0:
                    return
                metrics[metric_name].append(value)

            add_metric("keygen_us", "keygen_us", 5)
            add_metric("decaps_us", "decaps_us", 6)
            add_metric("handshake_us", "handshake_us", 7)
            add_metric("net_latency_us", "net_latency_us", 8)
            add_metric("jitter_us", "jitter_us", 9)
            add_metric("energy_uj", "energy_uj", 10)
            add_metric("reconnect_us", "reconnect_us", 11)
            add_metric("reconnect_energy_uj", "reconnect_energy_uj", 12)
            add_metric("cpu_util_percent", "cpu_util_percent", 22, skip_negative=False)

    packet_loss_rate = (loss_count / total) if total else None
    return (variant or "unknown").strip() or "unknown", metrics, packet_loss_rate


def compare_metric(off_values, on_values):
    off_stat = stats(off_values)
    on_stat = stats(on_values)
    if off_stat["p50"] is None or on_stat["p50"] is None:
        delta = None
        delta_percent = None
    else:
        delta = on_stat["p50"] - off_stat["p50"]
        delta_percent = (delta / off_stat["p50"] * 100) if off_stat["p50"] else None
    return off_stat, on_stat, delta, delta_percent


def build_s2_table(variant, off_data, on_data):
    lines = []
    lines.append("| variant | op | metric | PIE off (p50/p95) | PIE on (p50/p95) | delta | delta_percent |")
    lines.append("|---|---|---|---|---|---|---|")
    metrics = ["time_us", "cycles", "energy_uj"]
    ops = sorted(set(off_data.keys()) | set(on_data.keys()))
    for op in ops:
        for metric in metrics:
            off_vals = off_data.get(op, {}).get(metric, [])
            on_vals = on_data.get(op, {}).get(metric, [])
            off_stat, on_stat, delta, delta_percent = compare_metric(off_vals, on_vals)
            delta_str = fmt_num(delta) if delta is not None else "n/a"
            delta_pct = f"{delta_percent:.2f}%" if delta_percent is not None else "n/a"
            lines.append(
                f"| {variant} | {op} | {metric} | {fmt_p50_p95(off_stat)} | {fmt_p50_p95(on_stat)} | {delta_str} | {delta_pct} |"
            )
    return "\n".join(lines)


def build_s1_table(variant, off_metrics, on_metrics, off_loss, on_loss):
    lines = []
    lines.append("| variant | metric | PIE off (p50/p95) | PIE on (p50/p95) | delta | delta_percent |")
    lines.append("|---|---|---|---|---|---|")
    metric_order = [
        "keygen_us",
        "decaps_us",
        "handshake_us",
        "net_latency_us",
        "jitter_us",
        "energy_uj",
        "reconnect_us",
        "reconnect_energy_uj",
        "cpu_util_percent",
    ]
    for metric in metric_order:
        off_vals = off_metrics.get(metric, [])
        on_vals = on_metrics.get(metric, [])
        off_stat, on_stat, delta, delta_percent = compare_metric(off_vals, on_vals)
        delta_str = fmt_num(delta) if delta is not None else "n/a"
        delta_pct = f"{delta_percent:.2f}%" if delta_percent is not None else "n/a"
        lines.append(
            f"| {variant} | {metric} | {fmt_p50_p95(off_stat)} | {fmt_p50_p95(on_stat)} | {delta_str} | {delta_pct} |"
        )
    loss_delta = None if off_loss is None or on_loss is None else on_loss - off_loss
    loss_delta_pct = None if off_loss in (None, 0) or on_loss is None else (loss_delta / off_loss * 100)
    loss_delta_str = fmt_num(loss_delta) if loss_delta is not None else "n/a"
    loss_delta_pct_str = f"{loss_delta_pct:.2f}%" if loss_delta_pct is not None else "n/a"
    lines.append(
        "| {variant} | packet_loss_rate | {off} | {on} | {delta} | {delta_pct} |".format(
            variant=variant,
            off=fmt_num(off_loss, decimals=4),
            on=fmt_num(on_loss, decimals=4),
            delta=loss_delta_str,
            delta_pct=loss_delta_pct_str,
        )
    )
    return "\n".join(lines)


def write_output(text, out_path):
    if out_path:
        out_file = Path(out_path)
        out_file.parent.mkdir(parents=True, exist_ok=True)
        out_file.write_text(text, encoding="utf-8")
    else:
        print(text)


def main():
    parser = argparse.ArgumentParser(description="Compare PIE on/off CSV metrics and emit a summary table.")
    parser.add_argument("--scenario", choices=["s1", "s2"], required=True)
    parser.add_argument("--pie-off", required=True)
    parser.add_argument("--pie-on", required=True)
    parser.add_argument("--out")
    parser.add_argument("--format", choices=["md", "csv"], default="md")
    parser.add_argument("--drop-warmup", type=int, default=5)
    parser.add_argument("--include-loss-samples", action="store_true")
    args = parser.parse_args()

    if args.format != "md":
        raise SystemExit("Only md output is supported right now.")

    if args.scenario == "s2":
        variant_off, off_data = parse_s2(args.pie_off, args.drop_warmup)
        variant_on, on_data = parse_s2(args.pie_on, args.drop_warmup)
        variant = variant_off if variant_off != "unknown" else variant_on
        table = build_s2_table(variant, off_data, on_data)
    else:
        ignore_loss = not args.include_loss_samples
        variant_off, off_metrics, off_loss = parse_s1(args.pie_off, args.drop_warmup, ignore_loss)
        variant_on, on_metrics, on_loss = parse_s1(args.pie_on, args.drop_warmup, ignore_loss)
        variant = variant_off if variant_off != "unknown" else variant_on
        table = build_s1_table(variant, off_metrics, on_metrics, off_loss, on_loss)

    write_output(table, args.out)


if __name__ == "__main__":
    main()
