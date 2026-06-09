#!/usr/bin/env python3
import argparse
import csv
import re
from collections import defaultdict
from datetime import datetime
from pathlib import Path

FILENAME_RE = re.compile(r"^(s[12])_(512|768|1024)_(pieon|pieoff)_(reconn|session)_(\d{8}_\d{6})\.csv$")


def parse_ts(value: str) -> datetime:
    return datetime.strptime(value, "%Y%m%d_%H%M%S")


def index_files(folder: Path):
    groups = defaultdict(list)
    for path in sorted(folder.glob("*.csv")):
        match = FILENAME_RE.match(path.name)
        if not match:
            continue
        scenario, variant, pie, reconn, ts = match.groups()
        groups[(scenario, variant, pie, reconn)].append((parse_ts(ts), path))
    for key in groups:
        groups[key].sort(key=lambda item: item[0])
    return groups


def load_encaps_map(path: Path):
    encaps_map = {}
    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            if not row:
                continue
            try:
                iter_id = int((row.get("iter") or "").strip())
            except ValueError:
                continue
            encaps_us = (row.get("encaps_us") or "").strip()
            ss_hex = (row.get("ss_enc_hex") or row.get("ss_hex") or "").strip()
            encaps_map[iter_id] = (encaps_us, ss_hex)
    return encaps_map


def join_pair(esp_path: Path, sub_path: Path, out_path: Path):
    encaps_map = load_encaps_map(sub_path)
    missing = 0
    total = 0
    ss_dec_idx = None

    with esp_path.open("r", encoding="utf-8", newline="") as src, out_path.open(
        "w", encoding="utf-8", newline=""
    ) as dst:
        reader = csv.reader(src)
        writer = csv.writer(dst)
        for row in reader:
            if not row:
                continue
            if row[0].strip().lower() == "scenario":
                try:
                    ss_dec_idx = row.index("ss_dec_hex")
                except ValueError:
                    ss_dec_idx = None
                writer.writerow(row + ["encaps_us", "ss_enc_hex", "ss_valid"])
                continue
            if row[0].strip().lower() != "s1":
                continue
            try:
                iter_id = int(row[2])
            except (ValueError, IndexError):
                continue
            total += 1
            encaps_us, ss_enc_hex = encaps_map.get(iter_id, ("", ""))
            ss_dec_hex = ""
            if ss_dec_idx is not None and ss_dec_idx < len(row):
                ss_dec_hex = row[ss_dec_idx].strip()
            if encaps_us == "" or ss_enc_hex == "":
                missing += 1
            ss_valid = "1" if ss_dec_hex and ss_enc_hex and ss_dec_hex.lower() == ss_enc_hex.lower() else "0"
            writer.writerow(row + [encaps_us, ss_enc_hex, ss_valid])

    return total, missing


def main():
    parser = argparse.ArgumentParser(description="Join subscriber encaps_us into ESP32 Scenario 1 CSV.")
    parser.add_argument("--esp-dir", default="data/csv")
    parser.add_argument("--sub-dir", default="data/csv_subscriber")
    parser.add_argument("--out-dir", default="data/csv_joined")
    parser.add_argument("--scenario", choices=["s1"], default="s1")
    args = parser.parse_args()

    esp_dir = Path(args.esp_dir)
    sub_dir = Path(args.sub_dir)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    esp_groups = index_files(esp_dir)
    sub_groups = index_files(sub_dir)

    keys = sorted(set(esp_groups.keys()) & set(sub_groups.keys()))
    if not keys:
        raise SystemExit("No matching files found for join.")

    for key in keys:
        scenario, variant, pie, reconn = key
        if scenario != args.scenario:
            continue
        esp_list = esp_groups[key]
        sub_list = sub_groups[key]
        pairs = min(len(esp_list), len(sub_list))
        if pairs == 0:
            continue
        for idx in range(pairs):
            esp_ts, esp_path = esp_list[idx]
            sub_ts, sub_path = sub_list[idx]
            out_name = esp_path.stem + "_joined.csv"
            out_path = out_dir / out_name
            total, missing = join_pair(esp_path, sub_path, out_path)
            print(
                f"Joined {esp_path.name} + {sub_path.name} -> {out_path.name} (rows={total}, missing={missing})"
            )


if __name__ == "__main__":
    main()
