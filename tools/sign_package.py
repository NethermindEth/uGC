#!/usr/bin/python3
"""
Sign packages for bflat (docs/signing.md in bflat-riscv64).

    sign_package.py nupkg <package.nupkg>...
        Adds bflat-signature.sha256 and bflat-signature.sha256.sig to the root
        of each package. The list names every other entry of the package.

    sign_package.py files <out-dir> <file>...
        Writes bflat-signature.sha256 and bflat-signature.sha256.sig into
        <out-dir>. The list names each file by its base name: use it for the
        assets of a release.

The private key is the PEM text in the environment variable BFLAT_SIGNING_KEY
(an EC P-256 key: openssl ecparam -name prime256v1 -genkey -noout). openssl
makes the signature. The script stops with an error if the key is not set:
an unsigned release must not look signed.

The original is src/bflat/scripts/sign_package.py in bflat-riscv64, next
to PackageSignature.cs, which checks the signatures. bflat-libziskos,
bflat-sp1, bflat-openvm and dotnet-riscv have copies; keep them the same.
"""
import hashlib
import os
import subprocess
import sys
import tempfile
import zipfile

LIST_NAME = "bflat-signature.sha256"
SIGNATURE_NAME = "bflat-signature.sha256.sig"
# NuGet.org adds this after the push; bflat does not look for it in the list.
NUGET_SIGNATURE_NAME = ".signature.p7s"
KEY_VARIABLE = "BFLAT_SIGNING_KEY"


def fail(message):
    print(f"sign_package.py: {message}", file=sys.stderr)
    sys.exit(1)


def list_text(hashes):
    for name in hashes:
        if "\n" in name or "\r" in name:
            fail(f"a file name has a line break: {name!r}")
    return "".join(f"{hashes[name]}  {name}\n" for name in sorted(hashes)).encode("utf-8")


def sign(data):
    key = os.environ.get(KEY_VARIABLE, "")
    if not key.strip():
        fail(f"{KEY_VARIABLE} is not set")
    with tempfile.TemporaryDirectory() as tmp:
        key_path = os.path.join(tmp, "key.pem")
        fd = os.open(key_path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
        with os.fdopen(fd, "w") as f:
            f.write(key if key.endswith("\n") else key + "\n")
        result = subprocess.run(["openssl", "dgst", "-sha256", "-sign", key_path],
                                input=data, capture_output=True, check=False)
    if result.returncode != 0:
        fail("openssl could not sign: " + result.stderr.decode(errors="replace").strip())
    return result.stdout


def sign_nupkg(path):
    hashes = {}
    with zipfile.ZipFile(path) as zf:
        for info in zf.infolist():
            name = info.filename
            if name.endswith("/"):
                continue
            if name in (LIST_NAME, SIGNATURE_NAME):
                fail(f"{path} is signed already")
            if name == NUGET_SIGNATURE_NAME:
                fail(f"{path} has a NuGet repository signature; sign it before the push")
            if name in hashes:
                fail(f"{path} has the entry {name} two times")
            hashes[name] = hashlib.sha256(zf.read(info)).hexdigest()
    data = list_text(hashes)
    signature = sign(data)
    with zipfile.ZipFile(path, "a", compression=zipfile.ZIP_DEFLATED) as zf:
        zf.writestr(LIST_NAME, data)
        zf.writestr(SIGNATURE_NAME, signature)
    print(f"signed {path} ({len(hashes)} entries)")


def sign_files(out_dir, paths):
    hashes = {}
    for path in paths:
        name = os.path.basename(path)
        if name in hashes:
            fail(f"two files have the name {name}")
        digest = hashlib.sha256()
        with open(path, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                digest.update(chunk)
        hashes[name] = digest.hexdigest()
    data = list_text(hashes)
    signature = sign(data)
    os.makedirs(out_dir, exist_ok=True)
    with open(os.path.join(out_dir, LIST_NAME), "wb") as f:
        f.write(data)
    with open(os.path.join(out_dir, SIGNATURE_NAME), "wb") as f:
        f.write(signature)
    print(f"signed {len(hashes)} files into {out_dir}")


def main(argv):
    if len(argv) >= 2 and argv[0] == "nupkg":
        for path in argv[1:]:
            sign_nupkg(path)
    elif len(argv) >= 3 and argv[0] == "files":
        sign_files(argv[1], argv[2:])
    else:
        fail(__doc__.strip())
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
