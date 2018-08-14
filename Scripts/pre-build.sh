#!/bin/sh

echo "Running Pre-Build commands."

# sh "${ProjDirPath}/Scripts/pre-build.sh" "${ProjDirPath}" ${ConfigName} ${ProjName}
# argument $1 - path to project's directory
# argument $2 - build configuration name
# argument $3 - project name

PROJECT_DIR=$1
CONFIG_NAME=$2
PROJECT_NAME=$3

# Formatting

if command -v astyle >/dev/null 2; then
  ASTYLE=astyle
  echo "AStyle detected as ${ASTYLE}."
elif command -v $PROJECT_DIR/Scripts/astyle.exe >/dev/null 2; then
  ASTYLE=$PROJECT_DIR/Scripts/astyle.exe
  echo "AStyle detected as ${ASTYLE}."
else
  echo "AStyle not found. Aborting."
  exit 1;
fi

$ASTYLE --options=$PROJECT_DIR/Scripts/astyle.conf "$PROJECT_DIR/Middlewares/Riuson/*.*"

exit 0
