#!/bin/bash
# -x

FFMPEG_DIR="$(dirname "${BASH_SOURCE[0]}" )"
RUN_DIR="$(pwd)"
SAVE_DIR="$FFMPEG_DIR/.typo_check"

#ffmpeg last commit id
LAST_COMMIT_ID=$(git log -1 | sed -n '1p' |awk '{print $2}' )

#ffmpeg 4.4 orignal source code commit id (unmodified by amlogic)
BASE_COMMIT_ID=$(git log |grep "add ffmpeg4.4" -B 4 | sed -n '1p' |awk '{print $2}')
BASE_COMMIT_MSG=$(git log -1  $BASE_COMMIT_ID --raw )

#files modified by amlogic
MODIFIED_FILES=$(git diff --name-status $BASE_COMMIT_ID $LAST_COMMIT_ID |grep "^[MA]" | awk '{print $2}' ) #|sort -n |uniq -d

function init() {
    rm -rf "$SAVE_DIR"
    sync
    mkdir -p "$SAVE_DIR"
}

function is_base_added() {
    modified_file=$1
    base_added=$(echo $BASE_COMMIT_MSG |grep "$modified_file" |wc -l)
    if [ $((base_added)) -gt 0 ]; then
        return 1
    fi
    return 0
}

function get_first_commit_for_file() {
    target_file=$1
    is_base_added $target_file
    if [ $((base_added)) -eq 1 ]; then
        echo $BASE_COMMIT_ID
    else
        echo "$(git log --oneline $target_file |sed -n '$p' |awk '{print $1}')"
    fi
}

init
for file in $MODIFIED_FILES
do
    first_commit="$(get_first_commit_for_file $file)"
    diff_content=$(git diff $first_commit:$file $LAST_COMMIT_ID:$file)
    if [ $(($?)) -eq 0 ]; then
        #echo "ID:"$first_commit"    -- "$file
        echo "$diff_content" > $SAVE_DIR/$(echo $file |sed 's/\//_/g')".diff"
    fi
done
