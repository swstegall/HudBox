#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 ppa:username/ppa-name"
    exit 1
fi

PPA="$1"

# Extract username and ppa name
if [[ "$PPA" =~ ^ppa:([^/]+)/(.+)$ ]]; then
    USERNAME="${BASH_REMATCH[1]}"
    REPO="${BASH_REMATCH[2]}"
else
    echo "Invalid PPA format. Use: ppa:username/repo"
    exit 1
fi

# Detect Ubuntu codename
CODENAME=$(lsb_release -sc)

# Launchpad keyserver
KEYSERVER="keyserver.ubuntu.com"

# Fetch signing key fingerprint from Launchpad API
echo "Fetching key fingerprint for $PPA…"
FPR=$(curl -s "https://launchpad.net/~${USERNAME}/+archive/ubuntu/${REPO}" \
    | grep -oE '[A-F0-9]{16}' | head -n 1)

if [[ -z "$FPR" ]]; then
    echo "Error: Could not determine signing key fingerprint for $PPA"
    exit 1
fi

echo "Found fingerprint: $FPR"

# Prepare keyring path
KRDIR="/etc/apt/keyrings"
KRFILE="${KRDIR}/${USERNAME}-${REPO}.gpg"

sudo mkdir -p "$KRDIR"

echo "Importing key from keyserver and exporting in APT-compatible format…"

echo "Downloading PPA key directly from Launchpad…"

curl -fsSL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x${FPR}" \
    | gpg --dearmor | sudo tee "$KRFILE" >/dev/null

sudo chmod 644 "$KRFILE"

# Create .sources file
SRCFILE="/etc/apt/sources.list.d/${USERNAME}-${REPO}-${CODENAME}.sources"

echo "Writing APT sources file: $SRCFILE"

sudo tee "$SRCFILE" >/dev/null <<EOF
Types: deb
URIs: https://ppa.launchpadcontent.net/${USERNAME}/${REPO}/ubuntu/
Suites: ${CODENAME}
Components: main
Signed-By: ${KRFILE}
EOF

echo "Updating APT…"
sudo apt update

echo ""
echo "Done! You can now install packages from:"
echo "  $PPA"
echo ""
