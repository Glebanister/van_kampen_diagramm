# Van Kampen diagramm generator

Tool for generating Van Kampen diagramms.

## Usage

### Install graphviz

```bash
sudo apt-get install graphviz
sudo apt-get install graphviz-dev
```

### Clone this repository recursively

```bash
git clone https://github.com/Glebanister/van_kampen_diagramm --recursive
```

### Build

```bash
mkdir build && cd build
make
```

### Generate .dot file

```bash
./vankamp-vis -i <group-representation-path>
```

Will generate `vankamp-vis-out.dot`

Where `group-representation-path` - path to group representation file in a format corresponding to
[LangToGroup](https://github.com/YaccConstructor/LangToGroup)
format.

|        Option        | Param                                                              | Argument type |
|:--------------------:|:-------------------------------------------------------------------|---------------|
|    `-i, --input`     | Specify input file                                                 | string        |
|    `-o, --output`    | Specify custom output file (default:  vankamp-vis-out)             | string        |
| `-c, --cycle-output` | Set boundary cycle output file (default:    vankamp-vis-cycle.txt) | string        |
|  `-n, --no-shuffle`  | Do not shuffle representation before generation                    | -             |
|    `-q, --quiet`     | Do not log status to console                                       | -             |
|    `-l, --limit`     | Set cells limit                                                    | integer       |
|     `-h, --help`     | Print usage                                                        | -             |

Group representation format example:

```bash
any text before representation
f := FreeGroup( "f.1", "f.2", "f.3", "f.4", "f.5", "f.6", "f.7", "f.8", "f.9" )
g := f / [ (f.20)^(-1)*(f.1)^(-1)*f.20*f.1, (f.20)^(-1)*(f.2)^(-1)*f.20*f.2, (f.20)^(-1)*(f.3)^(-1)*f.20*f.3, (f.20)^(-1)*(f.4)^(-1)*f.20*f.4, (f.20)^(-1)*(f.5)^(-1)*f.20*f.5, (f.20)^(-1)*(f.6)^(-1)*f.20*f.6, (f.20)^(-1)*(f.7)^(-1)*f.20*f.7 ];
any text after representation
```

### Generate [format] file

```bash
dot -T[format] <your-diagramm-path> -o outfile.[format]
```

Supported formats list can be found at [graphviz.org](https://graphviz.org/doc/info/output.html)

## Example

```bash
$ head -c 40 out.txt
local f, g;
f := FreeGroup( "f.1", "f.2"
$ ./vankamp-vis -i out.txt
$ dot -Tsvg vankamp-vis-out.dot -o out.svg
```

Will generate Van Kampen diagramm for alphabet group representation in `out.txt` file.

![example](media/example.jpg)
