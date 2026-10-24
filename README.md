# Lab 2 — Performance Measurement and Analysis

**Курс:** Системне програмування  
**Тема:** Вимірювання та аналіз продуктивності + низькорівнева оптимізація  

---

## Програма

Транспонування квадратної матриці `float` розміром **2048×2048** (16 МБ).

Дві реалізації:
- **`transpose_naive`** — наївний подвійний цикл. Запис у матрицю B йде по стовпцях → багато cache misses
- **`transpose_blocked`** — блочне транспонування (tile size 64×64). Блок повністю вміщається в L1/L2 кеш → суттєво менше cache misses

---
## Структура репозиторію

```
lab2-sysProg/
├── src/
│   └── matrix_transpose.cpp
├── scripts/
│   └── measure_energy.sh
├── results/
│   ├── time_no_opt.txt
│   ├── time_opt.txt
│   ├── perf_stat_no_opt.txt
│   ├── perf_stat_opt.txt
│   ├── perf_report_no_opt.txt
│   ├── perf_report_opt.txt
│   ├── energy_no_opt.txt
│   └── energy_opt.txt
├── flamegraphs/
│   ├── flamegraph_no_opt.svg
│   └── flamegraph_opt.svg
├── Makefile
└── README.md
```

---

## Збірка і запуск

### Вимоги
- Linux / WSL2 (Ubuntu)
- `g++`, `make`
- `perf`
- FlameGraph (`git clone https://github.com/brendangregg/FlameGraph.git ~/FlameGraph`)

### Компіляція

```bash
make no_opt    # g++ -O0 -g
make opt       # g++ -O3 -march=native -g
make           # обидві версії
```

### Запуск

```bash
./transpose_no_opt
./transpose_opt
```

---

## Результати

### Час виконання (10 запусків)

| Версія | `-O0` | `-O3` | Прискорення |
|---|---|---|---|
| Naive | ~64 мс | ~35 мс | **1.8x** |
| Blocked | ~41 мс | ~19 мс | **2.2x** |

### `/usr/bin/time --verbose`

| Показник | `-O0` | `-O3` |
|---|---|---|
| User time | 1.28 с | 0.63 с |
| Wall clock | 1.33 с | 0.67 с |
| Major page faults | 1 | 0 |
| Context switches | 2 | 0 |

### `perf stat`

| Показник | `-O0` | `-O3` |
|---|---|---|
| task-clock | 1435 мс | 508 мс |
| page-faults | 12431 | 12432 |
| elapsed time | 1.45 с | 0.51 с |

### `perf report`

| Функція | `-O0` | `-O3` |
|---|---|---|
| `transpose_naive` | 61.18% | 44.04% (inlined) |
| `transpose_blocked` | 31.75% | 43.60% (inlined) |

### Енерговитрати (i3-1315U, TDP 15W)

| | `-O0` | `-O3` | Економія |
|---|---|---|---|
| Час | 1.193 с | 0.753 с | — |
| Потужність | 1.95 W | 1.95 W | — |
| **Енергія** | **2.33 Дж** | **1.47 Дж** | **37%** |

---

## Аналіз асемблеру

| | `-O0` | `-O3` |
|---|---|---|
| Змінні циклу | У стеку `[rbp-4]` | У регістрах `rax`, `rcx` |
| Обчислення адрес | `imul` | `lea` + `add` |
| Переміщення float | `movss` | `vmovss` (AVX) |
| Інлайнінг функцій | Ні | Так |
