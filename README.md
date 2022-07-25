# Meteors Detection Project

## Dependencies

This project use `ffmpeg-io` and `nrc2` projects as submodules:

```bash
git submodule update --init --recursive
```

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
 * `-DTAH_DETECT_EXE` [default=`ON`] {possible:`ON`,`OFF`}: compile the detection chain executable.
 * `-DTAH_VISU_EXE`   [default=`ON`] {possible:`ON`,`OFF`}: compile the visual tracking executable.
 * `-DTAH_CHECK_EXE`  [default=`ON`] {possible:`ON`,`OFF`}: compile the check executable.

## Short User Documentation

This project generates 3 different executables:
  - `meteor-detect`: meteors detection chain.
  - `meteor-visu`: visualization of the detected meteors.
  - `meteor-check`: validation of the detected meteors with the field truth.

The next sub-sections describe *how to use* the generated executables.

### Détection avec `meteor-detect`

Exécutable de la chaîne de détection de météores dans `./exe/meteor-detect`.

  * **Input1**  `-input_video`: vidéo sur laquelle on veut détecter des météores
  * **Input2**  `-output_frames` (optionnelle): path frames output
  * **Input3**  `-output_tracks` (optionnelle): path files stats (`tracks.txt` && `bounding_box.txt`)
  * **Output** `tracks.txt`: liste des météores "détectés"

Les options disponibles sont :

| **Argument**     | **Type** | **Defaut**  | **Necessaire** |**Description** |
| :---             | :---     | :---        |    :---        |:--- |
| `-input_video`   | str      | None        | Oui            |path vidéo source |
| `-output_frames` | str      | None        | Non            |path frames output for debug|
| `-output_tracks` | str      | ./debug     | Non            |path files stats (`tracks.txt` && `bounding_box.txt`) |
| `-start_frame`   | int      | 0           | Non            |image de départ dans la séquence |
| `-end_frame`     | int      | 200000      | Non            |dernière image de la séquence |
| `-skip_frames`   | int      | 0           | Non            |nombre d'images à sauter |
| `-light_min`     | int      | 55          | Non            |seuil bas filtrage lumineux |
| `-light_max`     | int      | 80          | Non            |seuil haut filtrage lumineux |
| `-surface_min`   | int      | 3           | Non            |surface min des CC en pixels |
| `-surface_max`   | int      | 1000        | Non            |surface max des CC en pixels |
| `-k`             | int      | 3           | Non            |nombre de voisins dans KPPV |
| `-r_extrapol`    | int      | 5           | Non            |rayon de recherche d'une CC dans le cas d'une extrapolation |
| `-d_line`        | int      | 25          | Non            |delta pour lequel un point est toujours considéré comme étant sur une droite |
| `-diff_deviaton` | float    | 4.0         | Non            |facteur de multiplication de l’écart type (l'erreur d'une CC doit être supérieure a `diff_deviation` x `ecart_type` pour être considéré en mouvement)|


### Visualisation avec `meteor-visu`

Exécutable de la visualisation de la détection des météores dans `./exe/meteor-visu`.

  * **Input1**               `-input_tracks` : vidéo sur laquelle on veut détecter des météores
  * **Input2**               `-input_video`  : ficher `tracks.txt` correspondant à la vidéo input
  * **Input3** (optionnelle) `-validation`   : fichier vérité terrain
  * **Output**               `-output_video` : si Inputs3, alors vidéo avec rectangles englobants en couleur (Vert = Vrai positif / Rouge = Faux positif),
                                               sinon vidéo avec rectangles englobants en plusieurs teintes de vert selon le niveau de confiance de la détection.

Les options disponibles sont :

| **Argument**    | **Type** | **Defaut** | **Necessaire** | **Description** |
| :---            | :---     | :---       |  :---          |:--- |
| `-input_tracks` | str      | None       |  Oui           |`tracks.txt` |
| `-input_video`  | str      | None       |  Oui           |vidéo source |
| `-output_video` | str      | "../"      |  Non           |path vidéo output |
| `-validation`   | str      | None       |  Non           |fichier contenant la vérité terrain de la séquence pour mettre les couleurs (Rouge = faux positif / Vert = vrai positif) |

Note : pour exécuter `./exe/meteor-visu`, il faut impérativement avoir lancé `./exe/meteor-detect` sur la même vidéo auparavant pour avoir les fichiers `tracks.txt` et `bouding_box.txt`.

### Vérification avec `meteor-check`

Exécutable de la vérification de la détection des météores sous format texte dans `./exe/meteor-check`.

  * **Input1**  `-input_tracks` : vidéo sur laquelle on veut détecter des météores
  * **Input2**  `-validation`   : fichier contenant la vérité terrain de la séquence
  * **Output**  `-output`       : path du dossier contenant `validation.txt`, par défaut dans le dossier courant

Les options disponibles sont :

| **Argument**    | **Type** | **Defaut** | **Necessaire** | **Description** |
| :---            | :---     | :---       |    :---        |:--- |
| `-input_tracks` | str      |  None      |   Oui          |`tracks.txt` |
| `-output`       | str      |  "../"     |   Non          |path du dossier contenant `validation.txt` |
| `-validation`   | str      |  None      |   Oui          |Fichier contenant la vérité terrain de la séquence |

Note : pour exécuter `./exe/meteor-check`, il faut impérativement avoir lancé `./exe/meteor-detect` sur la même vidéo auparavant pour avoir le fichier `tracks.txt`.

### Exemples d'utilisation

Télécharger une vidéo avec des météores ici: https://lip6.fr/adrien.cassagne/data/tauh/in/2022_05_31_tauh_34_meteors.mp4.

#### Step 1 : Détection de météores

```shell
./exe/meteor-detect -input_video ../2022_05_31_tauh_34_meteors.mp4
```

Pour avoir les frames en binaire pour debug : 

```shell
./exe/meteor-detect -input_video ../2022_05_31_tauh_34_meteors.mp4 -output_frames ./frames
```

#### Step 2 : Visualisation de la détection

Visualisation SANS vérité terrain :

```shell
./exe/meteor-visu -input_video ../2022_05_31_tauh_34_meteors.mp4 -input_tracks ./debug/assoconflicts/SB_55_SH_80/2022_05_31_tauh_34_meteors/tracks.txt -output_video ../.
```

Visualisation AVEC vérité terrain :

```shell
./exe/meteor-visu -input_video ../2022_05_31_tauh_34_meteors.mp4 -input_tracks ./debug/assoconflicts/SB_55_SH_80/2022_05_31_tauh_34_meteors/tracks.txt -output_video ../ -validation ../validation/2022_05_31_tauh_34_meteors.txt
```

#### Step 3 : Validation par fichier texte

Utiliser `meteor-check` avec les options suivantes :

```shell
./exe/meteor-check -input_tracks ./debug/assoconflicts/SB_55_SH_80/2022_05_31_tauh_34_meteors.txt/tracks.txt -validation ../validation/2022_05_31_tauh_34_meteors.txt
```

