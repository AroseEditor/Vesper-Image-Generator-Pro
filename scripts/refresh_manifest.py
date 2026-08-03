import json
import sys
import urllib.request
from pathlib import Path

MANIFEST = Path(__file__).resolve().parent.parent / "resources" / "models.json"

SOURCES = {
    "sd15-emaonly": [
        ("checkpoint", "stable-diffusion-v1-5/stable-diffusion-v1-5", "v1-5-pruned-emaonly.safetensors"),
    ],
    "sdxl-base-1.0": [
        ("checkpoint", "stabilityai/stable-diffusion-xl-base-1.0", "sd_xl_base_1.0.safetensors"),
    ],
    "flux1-schnell-q4ks": [
        ("diffusion_model", "city96/FLUX.1-schnell-gguf", "flux1-schnell-Q4_K_S.gguf"),
        ("clip_l", "comfyanonymous/flux_text_encoders", "clip_l.safetensors"),
        ("t5xxl", "comfyanonymous/flux_text_encoders", "t5xxl_fp8_e4m3fn.safetensors"),
        ("vae", "second-state/FLUX.1-schnell-GGUF", "ae.safetensors"),
    ],
}


def fetch_tree(repo):
    url = f"https://huggingface.co/api/models/{repo}/tree/main?recursive=true"
    with urllib.request.urlopen(url, timeout=60) as response:
        return json.load(response)


def lookup(tree, filename):
    for item in tree:
        if item.get("path") == filename and item.get("lfs"):
            return item["lfs"]["oid"], item["lfs"]["size"]
    raise SystemExit(f"could not resolve {filename}")


def main():
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    trees = {}
    changed = 0

    for model in manifest["models"]:
        sources = SOURCES.get(model["id"])
        if not sources:
            continue
        by_role = {role: (repo, name) for role, repo, name in sources}
        total = 0
        for entry in model["files"]:
            repo, name = by_role[entry["role"]]
            if repo not in trees:
                trees[repo] = fetch_tree(repo)
            sha, size = lookup(trees[repo], name)
            if entry["sha256"] != sha or entry["size_bytes"] != size:
                changed += 1
            entry["sha256"] = sha
            entry["size_bytes"] = size
            total += size
        model["total_size_bytes"] = total

    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"updated {changed} field(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
