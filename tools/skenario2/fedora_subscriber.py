import argparse
import importlib
import time
from pathlib import Path

import paho.mqtt.client as mqtt


def load_kem(variant: str):
    candidates = []
    if variant == "512":
        candidates = ["ml_kem_512", "kyber512"]
    elif variant == "768":
        candidates = ["ml_kem_768", "kyber768"]
    elif variant == "1024":
        candidates = ["ml_kem_1024", "kyber1024"]

    for name in candidates:
        try:
            return importlib.import_module(f"pqcrypto.kem.{name}")
        except Exception:
            continue

    raise RuntimeError("No compatible pqcrypto KEM module found")


def main():
    parser = argparse.ArgumentParser(description="ML-KEM MQTT subscriber (Fedora)")
    parser.add_argument("--broker", default="192.168.1.10", help="MQTT broker host")
    parser.add_argument("--port", type=int, default=1883, help="MQTT broker port")
    parser.add_argument("--username", default="", help="MQTT username")
    parser.add_argument("--password", default="", help="MQTT password")
    parser.add_argument("--scenario", choices=["s1"], default="s1")
    parser.add_argument("--variant", choices=["512", "768", "1024"], default="512")
    parser.add_argument("--pie", choices=["pieon", "pieoff"], default="pieoff")
    parser.add_argument("--mode", choices=["reconn", "session"], default="session")
    parser.add_argument("--topic-pubkey", default="pqc/handshake/pubkey")
    parser.add_argument("--topic-ciphertext", default="pqc/handshake/ciphertext")
    parser.add_argument("--csv-out", default="", help="Output CSV path (default: auto)")
    args = parser.parse_args()

    kem = load_kem(args.variant)

    if not args.csv_out or args.csv_out.lower() == "auto":
        ts = time.strftime("%Y%m%d_%H%M%S")
        root_dir = Path(__file__).resolve().parents[1]
        csv_dir = root_dir / "data" / "csv_subscriber"
        csv_dir.mkdir(parents=True, exist_ok=True)
        csv_path = csv_dir / f"{args.scenario}_{args.variant}_{args.pie}_{args.mode}_{ts}.csv"
    else:
        csv_path = Path(args.csv_out)
        csv_path.parent.mkdir(parents=True, exist_ok=True)

    print(f"Subscriber CSV: {csv_path}")
    csv_file = open(csv_path, "a", encoding="utf-8", newline="")
    if csv_file.tell() == 0:
        csv_file.write("iter,variant,encaps_us,ss_enc_hex\n")

    def on_connect(client, userdata, flags, reason_code, properties=None):
        client.subscribe(args.topic_pubkey, qos=1)

    def on_message(client, userdata, msg):
        payload = msg.payload
        if len(payload) < 4:
            return
        iter_id = int.from_bytes(payload[0:4], "little")
        pubkey = payload[4:]

        t0 = time.perf_counter_ns()
        ct, ss_enc = kem.encrypt(pubkey)
        t1 = time.perf_counter_ns()

        encaps_us = (t1 - t0) / 1000.0
        resp = iter_id.to_bytes(4, "little") + ct
        client.publish(args.topic_ciphertext, resp, qos=1, retain=False)

        line = f"{iter_id},{args.variant},{int(encaps_us)},{ss_enc.hex()}\n"
        csv_file.write(line)
        csv_file.flush()
        print(line.strip())

    client = mqtt.Client(protocol=mqtt.MQTTv5)
    if args.username:
        client.username_pw_set(args.username, args.password)

    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(args.broker, args.port, keepalive=60)
    client.loop_forever()


if __name__ == "__main__":
    main()
