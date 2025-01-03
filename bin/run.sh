#!/usr/bin/env zsh

# Accept case-insensitive mode by converting to uppercase
MODE=${@:$OPTIND:1}
MODE=${MODE:u}
BD="$(pwd)/$(dirname $0)/.."
ARTIFACT_FOLDER="${BD}/test/artifacts"
LOG_FILE_ERR="${ARTIFACT_FOLDER}/err.log"

set -eu
setopt +o nomatch
echo "Formatting source code"
clang-format -i --style=file:"${HOME}/config/.clang-format" src/*
echo "Formatting done"

rm -rf "$BD/build"
rm -rf "$ARTIFACT_FOLDER"

FLAGS="-Wall -Wextra -Werror -std=c2x -pedantic -fsanitize=address"
if [ "$(uname -s)" = "Linux" ]; then
    FLAGS="${FLAGS} -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_GNU_SOURCE"
fi

if [ "${MODE}" = "TEST" ]; then
    FLAGS="${FLAGS} -g -DTEST"
elif [ "${MODE}" = "" ]; then
    FLAGS="${FLAGS} -O3 -DLOG_LEVEL=LEVEL_INFO"
else
    echo "ERROR: invalid mode ${MODE} - allowed modes are"
    echo " * TEST"
    echo " * (none)"
    exit 1
fi

mkdir -p "$BD/build"

clang -o $BD/build/json_serializer $BD/src/main.c -I$BD/src $(echo ${FLAGS})

if [ "${MODE}" = "TEST" ]; then
    mkdir -p "${ARTIFACT_FOLDER}"
    "${BD}/build/json_serializer" 2>"${LOG_FILE_ERR}"
    RET_VAL=$?
    if [ ${RET_VAL} -ne 0 ]; then
        echo -e "\n\n\e[31mFAIL:\e[0m Execution interrupted with error code ${RET_VAL}.\n\n"
        exit ${RET_VAL}
    fi
    if [ -f "${LOG_FILE_ERR}" ]; then
        if [ "$(cat ${LOG_FILE_ERR})" = "" ]; then
            echo -e "\n-----\n\e[32mSUCCESS:\e[0m All tests passed.\n-----\n"
        else
            echo -e "\n-----\n\e[31mFAIL:\e[0m The content of ${LOG_FILE_ERR} follows.\n-----\n"
            cat "${LOG_FILE_ERR}"
        fi
    else
        echo -e "\n\n\e[31mApplication not run.\e[0m\n\n"
    fi
else
    "${BD}/build/json_serializer"
fi

