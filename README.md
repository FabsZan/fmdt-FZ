# FMDT -- *Fast Meteor Detection Toolbox*

## Contents

[1. Dependencies](#dependencies)  
[2. Compilation with CMake](#compilation-with-cmake)  
[3. User Documentation](#user-documentation)  
[3.1. Detection Executable](#detection-executable)  
[3.2. Visualization Executable](#visualization-executable)  
[3.3. Checking Executable](#checking-executable)  
[3.4. Max-reduction Executable](#max-reduction-executable)  
[3.5. Examples of use](#examples-of-use)  
[3.6. Input and Output Text Formats](#input-and-output-text-formats)  
[4. List of Contributors](#list-of-contributors)

## Dependencies

This project use `ffmpeg-io`, `nrc2`, `c-vector` and `aff3ct-core` projects as submodules:

```bash
git submodule update --init --recursive
```

If you want to enable text indications in generated videos/images (`--show-id` option), the `OpenCV` library is required.

## Compilation with CMake

```bash
mkdir build
cd build
cmake ..
make -j4
```

Example of optimization flags:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-Wall -funroll-loops -fstrict-aliasing -march=native"
```

**Tips**: on Apple Silicon M1 CPUs and with Apple Clang, use `-mcpu=apple-m1` instead of `-march=native`.

The `CMake` file comes with several options:
 * `-DFMDT_DETECT_EXE`     [default=`ON`]  {possible:`ON`,`OFF`}: compile the detection chain executable.
 * `-DFMDT_VISU_EXE`       [default=`ON`]  {possible:`ON`,`OFF`}: compile the visual tracking executable.
 * `-DFMDT_CHECK_EXE`      [default=`ON`]  {possible:`ON`,`OFF`}: compile the check executable.
 * `-DFMDT_MAXRED_EXE`     [default=`ON`]  {possible:`ON`,`OFF`}: compile the max reduction executable.
 * `-DFMDT_VID2IMG_EXE`    [default=`ON`]  {possible:`ON`,`OFF`}: compile the video to images converter executable.
 * `-DFMDT_DEBUG`          [default=`OFF`] {possible:`ON`,`OFF`}: build the project using debugging prints: these additional prints will be output on `stderr` and prefixed by `(DBG)`.
 * `-DFMDT_OPENCV_LINK`    [default=`OFF`] {possible:`ON`,`OFF`}: link with OpenCV library (required to enable `--show-id` option in `fmdt-visu` executable).
 * `-DFMDT_AFF3CT_RUNTIME` [default=`OFF`] {possible:`ON`,`OFF`}: link with AFF3CT runtime and produce multi-threaded detection executable (`fmdt-detect-rt`).

## User Documentation

This project generates different executables:
  - `fmdt-detect` (and `fmdt-detect-rt*` if `-DFMDT_AFF3CT_RUNTIME` is set to `ON`): meteors detection chain.
  - `fmdt-visu`: visualization of the detected meteors.
  - `fmdt-check`: validation of the detected meteors with the field truth.
  - `fmdt-maxred`: max reduction of grayscale pixels on a video.
  - `fmdt-vid2img`: convert video input files into a series of grayscale images.

The next sub-sections describe *how to use* the generated executables.

### Detection Executable

The meteors detection chain is located here: `./exe/fmdt-detect`.

The list of available arguments:

| **Argument**       | **Type** | **Default** | **Req** | **Description** |
| :---               | :---     | :---        | :---    | :--- |
| `--in-video`       | str      | None        | Yes     | Input video path where we want to detect meteors (supports also a path to a folder containing PGM images). |
| `--out-bb`         | str      | None        | No      | Path to the bounding boxes file required by `fmdt-visu` to draw detection rectangles. |
| `--out-frames`     | str      | None        | No      | Path of the output frames for debug (PGM format). |
| `--out-stats`      | str      | None        | No      | Path of the output statistics, only required for debugging purpose. |
| `--out-mag`        | str      | None        | No      | Path to the output file containing magnitudes of the tracked objects. |
| `--fra-start`      | int      | 0           | No      | First frame id (included) to start the detection in the video sequence. |
| `--fra-end`        | int      | 0           | No      | Last frame id (included) to stop the detection in the video sequence. If set to 0, read entire video. |
| `--fra-skip`       | int      | 0           | No      | Number of frames to skip. |
| `--light-min`      | int      | 55          | No      | Minimum light intensity hysteresis threshold (grayscale [0;255]). |
| `--light-max`      | int      | 80          | No      | Maximum light intensity hysteresis threshold (grayscale [0;255]). |
| `--surface-min`    | int      | 3           | No      | Minimum surface of the CCs in pixel. |
| `--surface-max`    | int      | 1000        | No      | Maximum surface of the CCs in pixel. |
| `-k`               | int      | 3           | No      | Number of neighbors in the k-nearest neighbor matching (k-NN algorithm). |
| `--max-dist`       | int      | 10          | No      | Maximum number of pixels between two images (k-NN algorithm). |
| `--r-extrapol`     | int      | 5           | No      | Search radius for CC extrapolation (piece-wise tracking). |
| `--angle-max`      | float    | 20.0        | No      | Tracking angle max between two consecutive meteor moving points (in degree). |
| `--diff-dev`       | float    | 4.0         | No      | Multiplication factor of the standard deviation (CC error has to be higher than `diff deviation` x `standard deviation` to be considered in movement). |
| `--track-all`      | bool     | -           | No      | By default the program only tracks `meteor` object type. If `--track-all` is set, all object types are tracked (`meteor`, `star` or `noise`). |
| `--fra-star-min`   | int      | 15          | No      | Minimum number of frames required to track a star. |
| `--fra-meteor-min` | int      | 3           | No      | Minimum number of frames required to track a meteor. |
| `--fra-meteor-max` | int      | 100         | No      | Maximum number of frames required to track a meteor. |
| `--video-buff`     | bool     | -           | No      | Bufferize all the video in global memory before executing the chain (for now it only works with `--in-video` as a folder of PGM images). |
| `--video-loop`     | int      | 1           | No      | Number of times the video is read in loop  (for now it only works with `--in-video` as a folder of PGM images). |
| `--ffmpeg-threads` | int      | 0           | No      | Select the number of threads to use to decode video input (in `ffmpeg`). If set to 0, `ffmpeg` chooses the number of threads automatically. |

Output text formats are detailed in the [Input and Output Text Formats](#input-and-output-text-formats) section.

### Visualization Executable

The meteors visualization program is located here: `./exe/fmdt-visu`.

The list of available arguments:

| **Argument**       | **Type** | **Default**    | **Req** | **Description** |
| :---               | :---     | :---           | :---    | :--- |
| `--in-video`       | str      | None           | Yes     | Input video path. |
| `--in-tracks`      | str      | None           | Yes     | The tracks file corresponding to the input video (generated from `fmdt-detect`). |
| `--in-bb`          | str      | None           | Yes     | The bounding boxes file corresponding to the input video (generated from `fmdt-detect`). |
| `--in-gt`          | str      | None           | No      | File containing the ground truth. |
| `--out-video`      | str      | "out_visu.mp4" | No      | Path of the output video (MPEG-4 format) with meteor tracking colored rectangles. If `--in-gt` is set then the bounding rectangles are red if *false positive* and green if *true positive*. If `--in-gt` is NOT set then the bounding rectangles are levels of green depending on the detection confidence. |
| `--out-frames`     | str      | None           | No      | Path of the output frames for debug (PPM format). |
| `--show-id`        | bool     | -              | No      | Show the object ids on the output video and frames. Requires to link with OpenCV library (`-DFMDT_OPENCV_LINK` CMake option). |
| `--nat-num`        | bool     | -              | No      | Natural numbering of the object ids, work only if `--show-id` is set. |
| `--only-meteor`    | bool     | -              | No      | Show only meteors. |
| `--ffmpeg-threads` | int      | 0              | No      | Select the number of threads to use to decode video input (in `ffmpeg`). If set to 0, `ffmpeg` chooses the number of threads automatically. |

**Note**: to run `fmdt-visu`, it is required to run `fmdt-detect` before and on the same input video. This will generate the required `tracks.txt` and `bounding_box.txt` files.

Input text formats are detailed in the [Input and Output Text Formats](#input-and-output-text-formats) section.

### Checking Executable

The meteors checking program is located here: `./exe/fmdt-check`.

The list of available arguments:

| **Argument**  | **Type** | **Default** | **Req** | **Description** |
| :---          | :---     | :---        | :---    | :--- |
| `--in-tracks` | str      | None        | Yes     | The track file corresponding to the input video (generated from `fmdt-detect`). |
| `--in-gt`     | str      | None        | Yes     | File containing the ground truth. |

**Note**: to run `fmdt-check`, it is required to run `fmdt-detect` before. This will generate the required `tracks.txt` file.

Input/output text formats are detailed in the [Input and Output Text Formats](#input-and-output-text-formats) section.

### Max-reduction Executable

The max-reduction generation program is located here: `./exe/fmdt-maxred`.

The list of available arguments:

| **Argument**       | **Type** | **Default** | **Req** | **Description** |
| :---               | :---     | :---        | :---    | :--- |
| `--in-video`       | str      | None        | Yes     | Input video path. |
| `--in-tracks`      | str      | None        | No      | The tracks file corresponding to the input video (generated from `fmdt-detect`). |
| `--in-gt`          | str      | None        | No      | File containing the ground truth. |
| `--out-frame`      | str      | None        | Yes     | Path of the output frame (PGM format). |
| `--fra-start`      | int      | 0           | No      | First frame id (included) to start the max-reduction in the video sequence. |
| `--fra-end`        | int      | 0           | No      | Last frame id (included) to stop the max-reduction in the video sequence. If set to 0, read entire video. |
| `--show-id`        | bool     | -           | No      | Show the object ids on the output video and frames, works only if `--in-tracks` is set. Requires to link with OpenCV library (`-DFMDT_OPENCV_LINK` CMake option). |
| `--nat-num`        | bool     | -           | No      | Natural numbering of the object ids, works only if `--show-id` is set. |
| `--only-meteor`    | bool     | -           | No      | Show only meteors. |
| `--ffmpeg-threads` | int      | 0           | No      | Select the number of threads to use to decode video input (in `ffmpeg`). If set to 0, `ffmpeg` chooses the number of threads automatically. |

### Video to Images Converter

The video to images converter program is located here: `./exe/fmdt-vid2img`.
It's main interest is to directly work on images, thus removing the overhead of the source decoding (achieved by `ffmpeg`).

The list of available arguments:

| **Argument**       | **Type** | **Default** | **Req** | **Description** |
| :---               | :---     | :---        | :---    | :--- |
| `--in-video`       | str      | None        | Yes     | Input video path. |
| `--out-frames`     | str      | None        | Yes     | Path of the output frames (PGM format). |
| `--fra-start`      | int      | 0           | No      | First frame id (included) to start the conversion. |
| `--fra-end`        | int      | 0           | No      | Last frame id (included) to stop the conversion. If set to 0, read entire video. |
| `--ffmpeg-threads` | int      | 0           | No      | Select the number of threads to use to decode video input (in `ffmpeg`). If set to 0, `ffmpeg` chooses the number of threads automatically. |

### Examples of use

Download a video sequence containing meteors here: https://lip6.fr/adrien.cassagne/data/tauh/in/2022_05_31_tauh_34_meteors.mp4.
These video sequence comes from IMCCE (*l'Observatoire de Paris*) and is the result of an airborne observation of the 2022 τ-Herculids.
More information about the 2022 τ-Herculids is available here: https://www.imcce.fr/recherche/campagnes-observations/meteors/2022the.

#### Step 1: Meteors detection

```shell
./exe/fmdt-detect --in-video ./2022_05_31_tauh_34_meteors.mp4
```

Write tracks and bounding boxes into text files for `fmdt-visu` and `fmdt-check`:

```shell
./exe/fmdt-detect --in-video ./2022_05_31_tauh_34_meteors.mp4 --out-bb ./out_detect_bb.txt > ./out_detect_tracks.txt
```

#### Step 2: Visualization

Visualization **WITHOUT** ground truth:

```shell
./exe/fmdt-visu --in-video ./2022_05_31_tauh_34_meteors.mp4 --in-tracks ./out_detect_tracks.txt --in-bb ./out_detect_bb.txt
```

Visualization **WITH** ground truth:

```shell
./exe/fmdt-visu --in-video ./2022_05_31_tauh_34_meteors.mp4 --in-tracks ./out_detect_tracks.txt --in-bb ./out_detect_bb.txt --in-gt ../validation/2022_05_31_tauh_34_meteors.txt
```

**Note**: by default, the resulting video will be written in the `./out_visu.mp4` file (this behavior can be overloaded with the `--out-video` argument).

#### Step 3: Offline checking

Use `fmdt-check` with the following arguments:

```shell
./exe/fmdt-check --in-tracks ./out_detect_tracks.txt --in-gt ../validation/2022_05_31_tauh_34_meteors.txt
```

#### Step 4: Max reduction

Use `fmdt-maxred` with the following arguments:

```shell
./exe/fmdt-maxred --in-video ./2022_05_31_tauh_34_meteors.mp4 --out-frame out_maxred.pgm
```

### Input and Output Text Formats

This section details the various text formats used by the toolchain.
For each text format, the `#` character can be used for comments (at the beginning of a new line).

#### Tracks: `stdout` of `fmdt-detect` / `--in-tracks` in `fmdt-visu` and `fmdt-check`

The tracks represent the detected objects in the video sequence.

```
# -------||---------------------------||---------------------------||---------
#  Track ||           Begin           ||            End            ||  Object
# -------||---------------------------||---------------------------||---------
# -------||---------|--------|--------||---------|--------|--------||---------
#     Id || Frame # |      x |      y || Frame # |      x |      y ||    Type
# -------||---------|--------|--------||---------|--------|--------||---------
    {id} ||  {fbeg} | {xbeg} | {ybeg} ||  {fend} | {xend} | {yend} || {otype}
```

* `{id}`: a positive integer value representing a unique track identifier.
* `{fbeg}`: a positive integer value representing the first frame in the video sequence when the track is detected.
* `{xbeg}`: a positive real value of the x-axis coordinate (beginning of the track).
* `{ybeg}`: a positive real value of the y-axis coordinate (beginning of the track).
* `{fend}`: a positive integer value representing the last frame in the video sequence when the track is detected.
* `{xend}`: a positive real value of the x-axis coordinate (end of the track).
* `{yend}`: a positive real value of the y-axis coordinate (end of the track).
* `{otype}`: a string of the object type, can be: `meteor`, `star` or `noise`.

#### Bounding Boxes: `--out-bb` in `fmdt-detect` / `--in-bb` in `fmdt-visu`

The bounding boxes can be output by `fmdt-detect` (with the `--out-bb` argument) and are required by `fmdt-visu`.
Each bounding box defines the area of an object, frame by frame.

Here is the corresponding line format:
```
{frame_id} {x_radius} {y_radius} {center_x} {center_y} {track_id} {is_extrapolated}
```
Each line corresponds to a frame and to an object, each value is separated by a space character.

#### Ground Truth: `--in-gt` in `fmdt-visu`, `fmdt-check` & `fmdt-maxred`

Ground truth file gives objects positions over time. Here is the expected text format of a line:

```
{object_type} {begin_frame} {begin_x} {begin_y} {end_frame} {end_x} {end_y}
```

`{object_type}` can be `meteor`, `star` or `noise`.
`{begin_frame}`, `{begin_x}`, `{begin_y}`, `{end_frame}`, `{end_x}`, `{end_y}` are positive integers.
Each line corresponds to an object and each value is separated by a space character.

#### Check Report: `stdout` in `fmdt-check`

The first part of `fmdt-check` `stdout` is a table where each entry corresponds to an object of the ground truth (GT):

```
# ---------------||---------------||-----------------||--------
#    GT Object   ||      Hits     ||    GT Frames    || Tracks
# ---------------||---------------||-----------------||--------
# -----|---------||--------|------||--------|--------||--------
#   Id |    Type || Detect |  GT  ||  Start |  Stop  ||      #
# -----|---------||--------|------||--------|--------||--------
 {oid} | {otype} ||   {dh} | {gh} || {staf} | {stof} ||   {nt}
```

* `{oid}`: a positive integer value representing a unique identifier of ground truth object.
* `{otype}`: a string of the object type, can be: `meteor`, `star` or `noise`.
* `{dh}`: a positive integer value of the number of frames when the object is detected (from the tracks, `--in-tracks`).
* `{gh}`: a positive integer value of the number of frame when the object is present (from the ground truth, `--in-gt`).
* `{staf}`: a positive integer value of the frame start (from the ground truth, `--in-gt`).
* `{stof}`: a positive integer value of the frame stop (from the ground truth, `--in-gt`).
* `{nt}`: a positive integer value of the number of tracks that match the ground truth object.

In a second part, `fmdt-check` `stdout` gives some statistics in the following format (`{pi}` stands for *positive integer* and `{pf}` for *positive float*):

```
Statistics:
  - Number of GT objs = ['meteor': {pi}, 'star': {pi}, 'noise': {pi}, 'all': {pi}]
  - Number of tracks  = ['meteor': {pi}, 'star': {pi}, 'noise': {pi}, 'all': {pi}]
  - True positives    = ['meteor': {pi}, 'star': {pi}, 'noise': {pi}, 'all': {pi}]
  - False positives   = ['meteor': {pi}, 'star': {pi}, 'noise': {pi}, 'all': {pi}]
  - True negative     = ['meteor': {pi}, 'star': {pi}, 'noise': {pi}, 'all': {pi}]
  - False negative    = ['meteor': {pi}, 'star': {pi}, 'noise': {pi}, 'all': {pi}]
  - Tracking rate     = ['meteor': {pf}, 'star': {pf}, 'noise': {pf}, 'all': {pf}]
```

* `Number of GT objs`: the number of objects from the ground truth.
* `Number of tracks`: the number of objects from the tracks (`fmdt-detect` output).
* `True positives`: number of detected objects that are in the ground truth (with the same type).
* `False positives`: number of detected objects that are not in the ground truth (or that have a different type).
* `True negative`: number of detected objects that are different from the current type of object. For instance, if we focus on `meteor` object type, the number of false negatives is the sum of all the objects in the tracks that are `star` or `noise`.
* `False negative`: number of non-detected objects (present in the ground truth and not present in the tracks).
* `Tracking rate`: the sum of detected hits on the sum of the ground truth hits. Range is between 1 (perfect tracking) and 0 (nothing is tracked). When there are more hits in a track than in the ground truth, the detected hits are the ground truth hits minus the extra hits of the track.

For each line, the `meteor`, `star` and `noise` object types are considered.
`all` stands for all types, sometime `all` can be mean-less.

## List of Contributors

This toolbox is developed by the [LIP6](https://www.lip6.fr/) laboratory ([ALSOC](https://www.lip6.fr/recherche/team.php?acronyme=ALSOC) team) of [Sorbonne University](https://www.sorbonne-universite.fr/) in Paris.
Any external contributions are more than welcome.

Here is the list of contributors:
 * Clara CIOCAN, *Master student*
 * Mathuran KANDEEPAN, *Master student*
 * Maxime MILLET, *PhD student*
 * Florian LEMAÎTRE, *PhD*
 * Arthur HENNEQUIN, *PhD*
 * Adrien CASSAGNE, *Associate professor*
 * Lionel LACASSAGNE, *Full professor*
