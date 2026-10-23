#!/bin/bash

PROGRAM=$1
TDP=15  # Вт, базовий TDP i3-1315U

# Читаємо CPU stats ДО запуску
read cpu user1 nice1 sys1 idle1 rest1 < <(grep '^cpu ' /proc/stat)
TIME_BEFORE=$(date +%s%N)

# Запускаємо програму
$PROGRAM > /dev/null 2>&1

# Читаємо CPU stats ПІСЛЯ
TIME_AFTER=$(date +%s%N)
read cpu user2 nice2 sys2 idle2 rest2 < <(grep '^cpu ' /proc/stat)

# Рахуємо час у секундах
ELAPSED=$(echo "scale=3; ($TIME_AFTER - $TIME_BEFORE) / 1000000000" | bc)

# Рахуємо CPU utilization
TOTAL1=$((user1 + nice1 + sys1 + idle1))
TOTAL2=$((user2 + nice2 + sys2 + idle2))
IDLE_DIFF=$((idle2 - idle1))
TOTAL_DIFF=$((TOTAL2 - TOTAL1))
CPU_LOAD=$(echo "scale=2; 100 * (1 - $IDLE_DIFF / $TOTAL_DIFF)" | bc)

# Рахуємо енергію
ENERGY=$(echo "scale=4; $TDP * ($CPU_LOAD / 100) * $ELAPSED" | bc)

echo "============================================"
echo "Program:        $PROGRAM"
echo "Elapsed time:   ${ELAPSED} s"
echo "CPU load:       ${CPU_LOAD}%"
echo "TDP:            ${TDP} W"
echo "Estimated power: $(echo "scale=2; $TDP * $CPU_LOAD / 100" | bc) W"
echo "Estimated energy: ${ENERGY} J"
echo "============================================"
