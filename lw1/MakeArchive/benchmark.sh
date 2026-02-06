#!/bin/bash

EXECUTABLE="/home/dmitriy.rybakov/study/PP/cmake-build-debug/lw1/MakeArchive/MakeArchive"
BASE_DIR="/home/dmitriy.rybakov/study/PP/lw1/MakeArchive"
INPUT_FILES="$BASE_DIR/input/adapter.pptx $BASE_DIR/input/mvxx.pptx $BASE_DIR/input/Потоки.pptx $BASE_DIR/input/Управление_памятью.pptx $BASE_DIR/input/command.pptx $BASE_DIR/input/Введение_в_ОС.pptx $BASE_DIR/input/Принципы_SOLID.pptx $BASE_DIR/input/Управление_памятью_на_практике.pptx $BASE_DIR/input/composite.pptx $BASE_DIR/input/Основные_концепции_ОС.pptx $BASE_DIR/input/Процессы.pptx $BASE_DIR/input/Файловые_системы.pptx $BASE_DIR/input/Планирование.pptx $BASE_DIR/input/Синхронизация_и_межпроцессная_коммуникация.pptx"
OUTPUT_ARCHIVE="output.tar"
MAX_PROCESSES=16

mkdir -p test_results
cd test_results

echo "Mode,Processes,Time(s)"

for i in {1..16}
do
    rm -f "$OUTPUT_ARCHIVE"

    TIME=$( { /usr/bin/time -f "%e" "$EXECUTABLE" -P $i "$OUTPUT_ARCHIVE" $INPUT_FILES; } 2>&1 )

    echo "Parallel,$i,$TIME"

    if [ ! -f "$OUTPUT_ARCHIVE" ]; then
        echo "WARNING: Archive not created for $i processes" >&2
    fi
done

cd ..
echo "Statistics collection complete!"