#!/usr/bin/python3
"""
Sign packages for bflat (docs/signing.md in bflat-riscv64).

A release job signs in three steps, so that the private key is only in a
small job that runs no build (.github/workflows/sign.yml in bflat-riscv64):

    sign_package.py list-nupkg <out-dir> <package.nupkg>...
        Writes <out-dir>/<package>.nupkg.bflat-signature.sha256: the list of
        the entries of each package. No key is necessary.

    sign_package.py list-files <out-dir> <file>...
        Writes <out-dir>/bflat-signature.sha256: the list of the files, each
        by its base name. Use it for the assets of a release.

    sign_package.py merge <out-dir> <list>...
        Writes <out-dir>/bflat-signature.sha256: one list of all the files of
        the lists that list-files wrote. A release whose assets come from
        more than one job has one list for each job, and one signature.

    (sign.yml signs each list: <list>.sig.)

    sign_package.py embed <signature-dir> <package.nupkg>...
        Makes sure that each package agrees with its list, and adds the list
        and its signature to the root of the package as
        bflat-signature.sha256 and bflat-signature.sha256.sig.

    sign_package.py check-files <signature-dir> <file>...
        Makes sure that the files agree with <signature-dir>/bflat-signature.sha256.

embed and check-files also check the signature if the environment variable
BFLAT_SIGNING_PUBLIC_KEY has the trusted public keys (base64 DER, separated
by commas, or PEM), as bflat does.

To sign on one machine with the private key:

    sign_package.py nupkg <package.nupkg>...
        Adds bflat-signature.sha256 and bflat-signature.sha256.sig to the root
        of each package. The list names every other entry of the package.

    sign_package.py files <out-dir> <file>...
        Writes bflat-signature.sha256 and bflat-signature.sha256.sig into
        <out-dir>.

The private key is the PEM text in the environment variable BFLAT_SIGNING_KEY
(an EC P-256 key: openssl ecparam -name prime256v1 -genkey -noout). openssl
makes and checks the signatures. The script stops with an error if the key is
not set: an unsigned release must not look signed.

The original is src/bflat/scripts/sign_package.py in bflat-riscv64, next
to PackageSignature.cs, which checks the signatures. bflat-libziskos,
bflat-sp1, bflat-openvm, dotnet-riscv and uGC have copies; keep them the same.
"""
import base64
import hashlib
import os
import re
import subprocess
import sys
import tempfile
import zipfile

LIST_NAME = "bflat-signature.sha256"
SIGNATURE_NAME = "bflat-signature.sha256.sig"
NUPKG_LIST_SUFFIX = ".bflat-signature.sha256"
# NuGet.org adds this after the push; bflat does not look for it in the list.
NUGET_SIGNATURE_NAME = ".signature.p7s"
KEY_VARIABLE = "BFLAT_SIGNING_KEY"
PUBLIC_KEYS_VARIABLE = "BFLAT_SIGNING_PUBLIC_KEY"


def fail(message):
    print(f"sign_package.py: {message}", file=sys.stderr)
    sys.exit(1)


def list_text(hashes):
    for name in hashes:
        if "\n" in name or "\r" in name:
            fail(f"a file name has a line break: {name!r}")
    return "".join(f"{hashes[name]}  {name}\n" for name in sorted(hashes)).encode("utf-8")


def openssl(args, data=None):
    return subprocess.run(["openssl"] + args, input=data, capture_output=True, check=False)


def sign(data):
    key = os.environ.get(KEY_VARIABLE, "")
    if not key.strip():
        fail(f"{KEY_VARIABLE} is not set")
    with tempfile.TemporaryDirectory() as tmp:
        key_path = os.path.join(tmp, "key.pem")
        fd = os.open(key_path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
        with os.fdopen(fd, "w") as f:
            f.write(key if key.endswith("\n") else key + "\n")
        result = openssl(["dgst", "-sha256", "-sign", key_path], data)
    if result.returncode != 0:
        fail("openssl could not sign: " + result.stderr.decode(errors="replace").strip())
    return result.stdout


def public_keys():
    text = os.environ.get(PUBLIC_KEYS_VARIABLE, "")
    if not text.strip():
        return []
    if "-----BEGIN" in text:
        values = [re.sub(r"\s", "", m) for m in
                  re.findall(r"-----BEGIN PUBLIC KEY-----(.*?)-----END PUBLIC KEY-----", text, re.S)]
    else:
        values = [v for v in re.split(r"[,\s]+", text) if v]
    try:
        keys = [base64.b64decode(v, validate=True) for v in values]
    except ValueError as e:
        fail(f"{PUBLIC_KEYS_VARIABLE} has a value that is not base64: {e}")
    if not keys:
        fail(f"{PUBLIC_KEYS_VARIABLE} is set but has no public key")
    return keys


def check_signature(data, signature, what):
    """Checks the signature with the trusted public keys, if there are any."""
    keys = public_keys()
    if not keys:
        print(f"{PUBLIC_KEYS_VARIABLE} is not set: the signature of {what} is not checked")
        return
    with tempfile.TemporaryDirectory() as tmp:
        sig_path = os.path.join(tmp, "list.sig")
        with open(sig_path, "wb") as f:
            f.write(signature)
        for i, key in enumerate(keys):
            key_path = os.path.join(tmp, f"key{i}.der")
            with open(key_path, "wb") as f:
                f.write(key)
            result = openssl(["dgst", "-sha256", "-verify", key_path, "-keyform", "DER", "-signature", sig_path], data)
            if result.returncode == 0:
                print(f"the signature of {what} is valid")
                return
    fail(f"the signature of {what} is not valid for a trusted key")


def nupkg_hashes(path):
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
    return hashes


def file_hashes(paths):
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
    return hashes


def read(path):
    try:
        with open(path, "rb") as f:
            return f.read()
    except OSError as e:
        fail(f"cannot read {path}: {e.strerror}")


def write(path, data):
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    with open(path, "wb") as f:
        f.write(data)


def add_to_nupkg(path, data, signature, count):
    with zipfile.ZipFile(path, "a", compression=zipfile.ZIP_DEFLATED) as zf:
        zf.writestr(LIST_NAME, data)
        zf.writestr(SIGNATURE_NAME, signature)
    print(f"signed {path} ({count} entries)")


def sign_nupkg(path):
    hashes = nupkg_hashes(path)
    data = list_text(hashes)
    add_to_nupkg(path, data, sign(data), len(hashes))


def sign_files(out_dir, paths):
    data = list_text(file_hashes(paths))
    signature = sign(data)
    write(os.path.join(out_dir, LIST_NAME), data)
    write(os.path.join(out_dir, SIGNATURE_NAME), signature)
    print(f"signed {len(paths)} files into {out_dir}")


def list_nupkg(out_dir, paths):
    for path in paths:
        out = os.path.join(out_dir, os.path.basename(path) + NUPKG_LIST_SUFFIX)
        hashes = nupkg_hashes(path)
        write(out, list_text(hashes))
        print(f"listed {path} ({len(hashes)} entries) in {out}")


def list_files(out_dir, paths):
    out = os.path.join(out_dir, LIST_NAME)
    write(out, list_text(file_hashes(paths)))
    print(f"listed {len(paths)} files in {out}")


def merge(out_dir, list_paths):
    hashes = {}
    for list_path in list_paths:
        for line in read(list_path).decode("utf-8").splitlines():
            m = re.fullmatch(r"([0-9a-f]{64})  (.+)", line)
            if not m:
                fail(f"{list_path} has a line that is not '<sha256>  <name>': {line!r}")
            if m.group(2) in hashes:
                fail(f"two lists name {m.group(2)}")
            hashes[m.group(2)] = m.group(1)
    out = os.path.join(out_dir, LIST_NAME)
    write(out, list_text(hashes))
    print(f"merged {len(list_paths)} lists ({len(hashes)} files) into {out}")


def embed(sig_dir, paths):
    for path in paths:
        list_path = os.path.join(sig_dir, os.path.basename(path) + NUPKG_LIST_SUFFIX)
        data = read(list_path)
        signature = read(list_path + ".sig")
        hashes = nupkg_hashes(path)
        if list_text(hashes) != data:
            fail(f"{path} does not agree with {list_path}")
        check_signature(data, signature, path)
        add_to_nupkg(path, data, signature, len(hashes))


def check_files(sig_dir, paths):
    list_path = os.path.join(sig_dir, LIST_NAME)
    data = read(list_path)
    signature = read(os.path.join(sig_dir, SIGNATURE_NAME))
    if list_text(file_hashes(paths)) != data:
        fail(f"the files do not agree with {list_path}")
    check_signature(data, signature, list_path)
    print(f"{len(paths)} files agree with {list_path}")


def main(argv):
    if len(argv) >= 2 and argv[0] == "nupkg":
        for path in argv[1:]:
            sign_nupkg(path)
    elif len(argv) >= 3 and argv[0] == "files":
        sign_files(argv[1], argv[2:])
    elif len(argv) >= 3 and argv[0] == "list-nupkg":
        list_nupkg(argv[1], argv[2:])
    elif len(argv) >= 3 and argv[0] == "list-files":
        list_files(argv[1], argv[2:])
    elif len(argv) >= 3 and argv[0] == "merge":
        merge(argv[1], argv[2:])
    elif len(argv) >= 3 and argv[0] == "embed":
        embed(argv[1], argv[2:])
    elif len(argv) >= 3 and argv[0] == "check-files":
        check_files(argv[1], argv[2:])
    else:
        fail(__doc__.strip())
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
