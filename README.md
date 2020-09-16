# Van Kampen diagramm generator

Tool for generating Van Kampen diagramms

## Usage

### Install graphviz

```bash
sudo apt-get install graphviz
sudo apt-get install graphviz-dev
```

### Build

```bash
mkdir build && cd build
make
```

### To generate .ps file

```bash
./build/van_kampen_diagramm -s SEED > diagramm.dot
```

Where

* [optional] SEED is random seed for generator. Seed will be will be randomly selected if not specified.

### To generate [format] file

```bash
dot -T[format] diagramm.dot -o outfile.[format]
```

Supported formats list can be found at [graphviz.org](https://graphviz.org/doc/info/output.html)

## Example

```bash
./build/van_kampen_diagramm "<ab|aba-b->" -K dot > diagramm.dot
dot -Tpng diagramm.dot -o outfile.png
```

Will generate Van Kampen diagramm for alphabet `ab` and following words.
