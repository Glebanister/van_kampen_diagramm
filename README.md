# Van Kampen diagramm generator

Tool for generating Van Kampen diagramms. Does not work correctly at this moment.

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

### To generate .dot file

```bash
./build/van_kampen_diagramm "<a, b | aba*b*>" > diagramm.dot
```

### To generate [format] file

```bash
dot -T[format] diagramm.dot -o outfile.[format]
```

Supported formats list can be found at [graphviz.org](https://graphviz.org/doc/info/output.html)

## Example

```bash
./build/van_kampen_diagramm "<a, b | aba*b*, ab*a*b>" > build/diagramm.dot
dot -Tpng build/diagramm.dot -o outfile.png
```

Will generate Van Kampen diagramm for alphabet `ab` and following word.

![example](media/outfile.png)
