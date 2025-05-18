#!/bin/zsh
set -eu
setopt +o nomatch

APP_NAME="main-test"
ARTIFACT_FOLDER="test/artifacts"
LOG_FILE_ERR="${ARTIFACT_FOLDER}/err.log"
BUILD_DIR="build"
BD="$(pwd)/$(dirname $0)/.."

# Accept case-insensitive mode by converting to uppercase
MODE=${@:$OPTIND:1}
MODE=${MODE:u}

FLAGS=(-Wall -Wextra -std=c2x -pedantic)
if [ "${MODE}" = "DEBUG" ] || [ "${MODE}" = "TEST" ] ; then
    FLAGS+=(-O0 -g -fsanitize=address -DTEST -DMEMORY_CHECK)
fi

if [ "$(uname -s)" = "Linux" ]; then
    FLAGS+=(-D_BSD_SOURCE -D_DEFAULT_SOURCE -D_GNU_SOURCE)
fi


function f_analyze_mem() {
    echo "Memory report analysis started."
    echo
    if [ $(ls /tmp/pointers | wc -l) -ne 0 ]; then
        echo "\e[33mFAIL:\e[0m Memory leak detected."
        for f in $(ls /tmp/pointers); do
            echo "$f - $(cat /tmp/pointers/$f)"
        done
    else
        echo "\e[32mSUCCESS:\e[0m No memory leak detected."
    fi
    set -e
    echo
    echo "Memory report analysis completed."
}

pushd "${BD}"
echo "Closing running instance"
set +e
echo "${BD}/test/artifacts"
rm -rf "${BD}/test/artifacts"
rm -rf /tmp/pointers
rm -rf ${ARTIFACT_FOLDER}
mkdir -p "${BUILD_DIR}" 

PID=$(pgrep ${APP_NAME})
set -e
if ! [ "${PID}" = "" ]; then
    echo $PID
    kill ${PID}
else
    echo "No process was running."
fi

clang src/"${APP_NAME}.c" ${FLAGS} -o "${BUILD_DIR}/${APP_NAME}"

echo "Running"
if [ "${MODE}" = "TEST" ] || [ "${MODE}" = "DEBUG" ]; then
    mkdir -p /tmp/pointers
    # Set up dir entries for testing.
    mkdir -p "${ARTIFACT_FOLDER}/empty/" \
        "${ARTIFACT_FOLDER}/non-empty/inner/inner_l2" \
        "${ARTIFACT_FOLDER}/non-empty-0/inner/inner_l2" \
        "${ARTIFACT_FOLDER}/empty-0"

    touch "${ARTIFACT_FOLDER}/non-empty/inner/file.txt"
    touch "${ARTIFACT_FOLDER}/non-empty/inner/inner_l2/file.txt"
    touch "${ARTIFACT_FOLDER}/delete_me.txt"
    touch "${ARTIFACT_FOLDER}/delete_me_2.txt"

    if [ "${MODE}" = "TEST" ]; then
        # Remove previous logs.
        ./"${BUILD_DIR}/${APP_NAME}" 2>"${LOG_FILE_ERR}"
        RET_VAL=$?
        echo "================================================================================"
        if [ ${RET_VAL} -ne 0 ]; then
            echo -e "\n\n\e[31mFAIL:\e[0m Execution interrupted with error code ${RET_VAL}.\n\n"
            exit ${RET_VAL}
        fi
        if [ -f "${LOG_FILE_ERR}" ]; then
            if [ "$(cat ${LOG_FILE_ERR})" = "" ]; then
                echo -e "\n\n\e[32mSUCCESS:\e[0m All tests passed.\n\n"
            else
                echo -e "\n\n\e[31mFAIL:\e[0m The content of ${LOG_FILE_ERR} follows.\n\n"
                cat "${LOG_FILE_ERR}"
            fi
        else
            echo -e "\n\n\e[31mApplication not run.\e[0m\n\n"
        fi
        echo "================================================================================"
        f_analyze_mem
        echo
    else
        lldb ./"${BUILD_DIR}/${APP_NAME}"
    fi
elif [ "${MODE}" = "BUILD" ]; then
    echo "Build completed"    
else
    echo "Error: accepted mode is 'test', 'debug', or 'build'"
    exit 1
fi
popd
