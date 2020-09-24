# Van Kampen diagramm generator

Tool for generating Van Kampen diagramms.

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
./build/van_kampen_diagramm <group_representation_path> > diagramm.dot
```

Where:

* `group_representation_path` - path to group representation file in a format corresponding to
[LangToGroup](https://github.com/YaccConstructor/LangToGroup)
format.

Group representation example:

```bash
any text before representation
f := FreeGroup( "f.1", "f.2", "f.3", "f.4", "f.5", "f.6", "f.7", "f.8", "f.9" )
g := f / [ (f.20)^(-1)*(f.1)^(-1)*f.20*f.1, (f.20)^(-1)*(f.2)^(-1)*f.20*f.2, (f.20)^(-1)*(f.3)^(-1)*f.20*f.3, (f.20)^(-1)*(f.4)^(-1)*f.20*f.4, (f.20)^(-1)*(f.5)^(-1)*f.20*f.5, (f.20)^(-1)*(f.6)^(-1)*f.20*f.6, (f.20)^(-1)*(f.7)^(-1)*f.20*f.7 ];
any text after representation
```

### To generate [format] file

```bash
dot -T[format] diagramm.dot -o outfile.[format]
```

Supported formats list can be found at [graphviz.org](https://graphviz.org/doc/info/output.html)

## Example

```bash
$ head -c 40 out.txt
local f, g;
f := FreeGroup( "f.1", "f.2"
$ ./van_kampen_diagramm out.txt
$ dot -Tpng ../../LangToGroup/out.txt.dot -o out.png
```

Will generate Van Kampen diagramm for alphabet group representation in `out.txt` file.

![example](media/example.jpg)
