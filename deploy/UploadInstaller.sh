#!/bin/bash

# receives file with json config as argument

# script dir (not calling or symlink dir)
dirname="$(dirname "$(readlink -f "$0")")"

# check os
uname_os="$(uname -s)"
case "${uname_os}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Win;;
    MINGW*)     machine=Win;;
	MSYS*)      machine=Win;;
    *)          machine="UNKNOWN:${uname_os}"
esac
echo "[INFO] os is ${machine}."

# test file exists
if [[ ! $1 ]]; then
    echo "\n[ERROR] Missing argument, expected JSON config file."
    exit 1
fi
if [[ ! -e $1 ]]; then
    printf "\n[ERROR] Config file :\n%s \ndoes not exist!\n\n" $1
    exit 1
fi
# read config from file and validate
GH_JSON_CONF=$(cat $1)
GH_USER=$(echo ${GH_JSON_CONF} | jq -r '.GH_USER')
GH_REPO=$(echo ${GH_JSON_CONF} | jq -r '.GH_REPO')
GH_TOKEN=$(echo ${GH_JSON_CONF} | jq -r '.GH_TOKEN')
GH_RELEASE_TAG=$(echo ${GH_JSON_CONF} | jq -r '.GH_RELEASE_TAG')
GH_RELEASE_NAME=$(echo ${GH_JSON_CONF} | jq -r '.GH_RELEASE_NAME')
GH_RELEASE_DESCRIP=$(echo ${GH_JSON_CONF} | jq -r '.GH_RELEASE_DESCRIP')
if [[ -z "$GH_USER" ]]; then
    printf "\n[ERROR] Config file :\n%s \ndoes not contain GH_USER.\n\n" $1
    exit 1
fi
if [[ -z "$GH_REPO" ]]; then
    printf "\n[ERROR] Config file :\n%s \ndoes not contain GH_REPO.\n\n" $1
    exit 1
fi
if [[ -z "$GH_TOKEN" ]]; then
    printf "\n[ERROR] Config file :\n%s \ndoes not contain GH_TOKEN.\n\n" $1
    exit 1
fi
if [[ -z "$GH_RELEASE_TAG" ]]; then
    printf "\n[ERROR] Config file :\n%s \ndoes not contain GH_RELEASE_TAG.\n\n" $1
    exit 1
fi
if [[ -z "$GH_RELEASE_NAME" ]]; then
    printf "\n[ERROR] Config file :\n%s \ndoes not contain GH_RELEASE_NAME.\n\n" $1
    exit 1
fi
if [[ -z "$GH_RELEASE_DESCRIP" ]]; then
    printf "\n[ERROR] Config file :\n%s \ndoes not contain GH_RELEASE_DESCRIP.\n\n" $1
    exit 1
fi

# get platform dependent artifact
if [[ $machine == "Win" ]]; then
	GH_ARTIFACT_EXT="exe"
else
	GH_ARTIFACT_EXT="AppImage"
fi
GH_ARTIFACT=$(find ${dirname}/.. -name "*.${GH_ARTIFACT_EXT}*")
if [[ ! -e ${GH_ARTIFACT} ]]; then
    printf "\n[ERROR] Artifact file :\n%s \ndoes not exist!\n\n" ${GH_ARTIFACT}
    exit 1
fi
GH_ARTIFACT_BASE=$(basename ${GH_ARTIFACT})

# get release
GH_REPO_URL=https://api.github.com/repos/${GH_USER}/${GH_REPO}
GH_REPO_RELEASES_URL=${GH_REPO_URL}/releases
GH_RELEASES=$(curl ${GH_REPO_RELEASES_URL})
GH_RELEASES_COUNT=$(echo $GH_RELEASES | jq -r 'length')
GH_RELEASE_ID=$(echo $GH_RELEASES | jq -r '.[0] | .id')

# if not exists then create
if [[ ${GH_RELEASES_COUNT} -eq 0 ]]; then
	GH_RELEASE_CREATE=$(curl -XPOST -H "Authorization:token ${GH_TOKEN}" --data "{\"tag_name\": \"${GH_RELEASE_TAG}\", \"target_commitish\": \"master\", \"name\": \"${GH_RELEASE_NAME}\", \"body\": \"${GH_RELEASE_DESCRIP}\", \"draft\": false, \"prerelease\": true}" ${GH_REPO_RELEASES_URL})
	GH_RELEASE_ID=$(echo ${GH_RELEASE_CREATE} | jq -r '.id')
fi

# update release tag with latest master commit
GH_CURR_SHA=$(git rev-parse HEAD)
curl -XPATCH -H "Authorization:token ${GH_TOKEN}" --data "{\"sha\": \"${GH_CURR_SHA}\", \"force\": true}" ${GH_REPO_URL}/git/refs/tags/${GH_RELEASE_TAG}

# read current artifacts
GH_ASSETS=$(curl ${GH_REPO_RELEASES_URL}/${GH_RELEASE_ID}/assets)
GH_ASSETS_COUNT=$(echo $GH_ASSETS | jq -r 'length')
if [[ ${GH_ASSETS_COUNT} -gt 2 ]]; then
	printf "\n[ERROR] Too many existing artifacts.\n\n"
    exit 1
fi

# delete old artifact if exists
GH_ARTIFACT_OLD=$(echo $GH_ASSETS | jq -r '.[] | select(.name | contains("${GH_ARTIFACT_EXT}")) | .id')
if [[ ! -z "$GH_ARTIFACT_OLD" ]]; then
    curl --request DELETE -H "Authorization:token ${GH_TOKEN}" ${GH_REPO_RELEASES_URL}/assets/${GH_ARTIFACT_OLD}
fi

# upload new artifact
GH_REPO_UPDLOAD_URL=https://uploads.github.com/repos/${GH_USER}/${GH_REPO}/releases/${GH_RELEASE_ID}/assets?name=${GH_ARTIFACT_BASE}
GH_REPO_UPDLOAD=$(curl -XPOST -H "Authorization:token ${GH_TOKEN}" -H "Content-Type:application/octet-stream" --data-binary @${GH_ARTIFACT} ${GH_REPO_UPDLOAD_URL})

# check
GH_OK=$(echo $GH_REPO_UPDLOAD | jq -r '.id')
if [[ -z "$GH_OK" ]]; then
    printf "\n[ERROR] Failed to upload artifact %s.\n\n" ${GH_ARTIFACT}
    exit 1
fi