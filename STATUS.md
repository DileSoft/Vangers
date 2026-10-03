# Состояние ветки и следующий шаг

Обновлено 2026-10-03. Продолжение поэтапного мержа, стадия 4 (SDL3).

## Где мы

Ветка `wip-js-runtime-staged-merge` от `backup/pre-source-master-merge`
(`22a66cdb`). **Не запушена.**

```
2b451a76 Merge branch 'up-s3' into wip-js-runtime-staged-merge
c8cd97a0 Merge upstream stage 2: header self-containment, ...
09a40ddc Record upstream's mass clang-format sweep as merged, ...
abd39d71 Restore vcpkg.json so the MSVC build can configure
29cd32e5 Normalise line endings to LF on our side before merging
22a66cdb (backup/pre-source-master-merge)
```

Стадия 4 в процессе. Из ~200 хунков осталось **68** в трёх файлах.

## Главная причина конфликтов (установлена)

Наша ветка осталась в **базовом форматировании** (brace на следующей строке,
`char*` без пробела), upstream перешёл на **clang-format**. Поэтому почти
каждый наш изменённый файл отличается от upstream'а и по существу, и по
стилю — и git конфликтует.

Проверено: в файлах без наших фич (`i_chat.cpp`, `actintml.cpp`, `ikeys.cpp`,
`xjoystick.cpp`, `avi.cpp`, `xsound.cpp`, `controls.cpp`, `xsocket.h`) наши
уникальные строки — это **старые SDL2-версии** тех же конструкций
(`extern char *` против `const char*`, `SDL_net.h`, `GameController`,
`AV_CODEC_PAR`). Содержательного смысла в них нет.

**Практический вывод:** там, где скан не находит наших фич, верный ответ —
взять upstream целиком. Так были закрыты 8 файлов (51 хунк) за одну операцию.

Сканер фич (искать в `HEAD:<file>`):

```
vss|normal_loop|emscripten|readyQuant|tickQuant|runtimeObjectQuant|readyObject|
AbstractCompositor|renderer::|iOptionsDataLoading|FrameModelHandles|
ExternalModel|BindExternalModel|SyncExternalModel|external_body_color|
__use_external_renderer|request_region_update|libopfs|vange-rs|glad::|SCRIPT
```

## Что сделано в стадии 4

Система сборки приведена к SDL3:
- `CMakeLists.txt`: `-sUSE_SDL=3`, `FIND_PACKAGE(SDL3 3.2 CONFIG)`,
  `SDL3::SDL3` / `SDL3::SDL3_net`; добавлен `ADD_SUBDIRECTORY("tests")`
- `xtool`/`xgraph`/`xsound` переведены на схему upstream (ANDROID→OBJECT,
  иначе STATIC). Из-за этого убраны `$<TARGET_OBJECTS:...>` из не-Android
  ветки `src/CMakeLists.txt` (выражение только для OBJECT) и добавлен `glad`
- Подсистема XRecorder удалена целиком, как у upstream: `xrec.h`,
  `xrecorder/`, `RecorderMode`, `#include "xrec.h"` из `xglobal.h`

Закрытые файлы: `palette.cpp`, `vmap.cpp`, `dynamics.cpp`, `mechos.cpp`,
`network.h`, `network.cpp`(частично), `ikeys.h`, `ikeys.cpp`, `iscreen.cpp`,
`iscr_fnc.cpp`, `xjoystick.h/.cpp`, `xsocket.h/.cpp`, `xglobal.h`,
`xside.h/.cpp`, `xbmp.h/.cpp`, `xcritical.h`, `xtcore.h/.cpp`, `avi.h/.cpp`,
`xsound.cpp`, `ogg_stream.cpp`, `xclock.cpp`, `xerrhead.cpp`, `runtime.h`,
`aci_scr.cpp`, `ascr_fnc.cpp`, `sound.cpp`, `iextern.cpp`, `CMakeLists.txt`
(root и все subdir).

## Решения, которые важно не потерять

- **`xtcore.cpp`**: взята переписанная upstream `main()`; наши vss-вызовы
  сохранены. `sys_initScripts(argv[i])`, `sys_readyQuant()`,
  `sys_tickQuant()` пережили автослияние сами. Потерялся только
  по-итерационный `sys_runtimeObjectQuant(XObj->ID)` — возвращён вручную
  после `XObj = xtGetRuntimeObject(id);`. Наш `normal_loop()` /
  `em_normal_loop()` **не** восстанавливались: upstream делает то же самое
  инлайном, а emscripten-сборка всё равно не проверялась.
- **`network.h`**: `GLOBAL_CLOCK`/`LOCAL_CLOCK` и `START_TIMER` взяты из
  upstream — по условию задачи тайминги в main корректны. Сохранён наш
  `#ifdef EMSCRIPTEN` с `html::emSleep()` внутри `CHECK_TIMER`, но с
  `Uint64` из upstream.
- **`iscr_fnc.cpp`**: снят `if(!RecorderMode)` (XRecorder удалён), но
  **наш блок localStorage** (`iOptionsDataLoading = 1; ... iSetOptionValue`)
  возвращён в `iLoadData` вручную — при «взять theirs» он пропал.
- **`xgraph.cpp`**: наш compositor против их `SDL_Renderer` — в таких хунках
  берётся ours. Окно событий слито: их audio-pause + наш
  `SDL_EVENT_WINDOW_RESIZED` с `compositor->set_viewport(...)` и
  `VisualBackendContext::...->set_screen_resolution(...)`.
- **Палитра (`xgraph.cpp`)**: там же наш порядок байт R,G,B,A. Не брать
  theirs вслепую.

## Метод работы и две ошибки

`edit` не может надёжно выразить многострочный блок с ведущими табами —
проверено, отказы воспроизводимы. Однострочные замены работают. Поэтому для
блоков с табами используется построчный разбор с проверкой маркеров и баланса
скобок; `edit` — для одиночных строк.

Ошибки, которые пришлось исправлять:
1. Скрипт применился к первому хунку вместо нужного (главный цикл вместо
   window-событий) в `xtcore.cpp`. Восстановлено через `git merge-file`
   вручную: `git merge-file --diff3 ours base theirs`, где base = `up-s3`.
2. `git add -A` во время мера стёр информацию о стадиях, из-за чего
   `git checkout --ours/--theirs` стал недоступен. Стороны берутся через
   `git checkout HEAD -- <file>` и `git checkout up-sN -- <file>`.

## Осталось

| Файл | Хунков | Сложность |
|---|---|---|
| `lib/xgraph/xgraph.cpp` | 14 | Высокая: наш compositor против `SDL_Renderer`, палитра |
| `src/network.cpp` | 22 | Средняя: vss-хуки против SDL3_net |
| `src/road.cpp` | 24 | Высокая: GPU-блок отрисовки, `vss::`, `RecorderMode` |

## Сборка

Ветка до стадии 4 — SDL2, локально установлен SDL3, поэтому стадии 1-3 не
собираются и это ожидаемо. Сборка должна заработать на стадии 4.

```
vcvars64.bat && set PATH=%USERPROFILE%\.cargo\bin;%PATH% && ninja -C build-alpha vangers
```

- vcvars: `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat`
- ninja: `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe`
- clunk: `FIND_PACKAGE(Clunk REQUIRED)` против `external/clunk-install`

## Ограничения, заданные пользователем

1. **В `source/master` физика и тайминги работают корректно.** Не «исправлять»
   без доказательств, что виновата наша сторона.
2. Возможно, причина в сочетании правок обеих веток.
3. **Сначала мерж с минимальным числом конфликтов. Баги — потом.**

## Документы

| Файл | Содержание |
|---|---|
| `MERGE_PLAN_STAGED.md` | План: 8 стадий, разбор 116 коммитов |
| `STATUS.md` | Этот файл |
| `bugs.md` | Открытые баги |
| `MERGE_NOTES.md` | Справочный: ручные разрешения первой попытки |