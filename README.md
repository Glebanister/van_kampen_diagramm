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

|        Option        | Param                                                                 | Argument type        |
|:--------------------:|:----------------------------------------------------------------------|----------------------|
|    `-i, --input`     | Specify input file                                                    | string               |
|    `-o, --output`    | Specify custom output file (default:  `<input-filename>-diagram.dot`) | string               |
| `-c, --cycle-output` | Set boundary cycle output file (default:    vankamp-vis-cycle.txt)    | string               |
|  `-n, --no-shuffle`  | Do not shuffle representation before generation                       | -                    |
|    `-q, --quiet`     | Do not log status to console                                          | -                    |
|    `-l, --limit`     | Set cells limit                                                       | non-negative integer |
|    `--per-large`     | Set the number of small words used to build one big one               | non-negative integer |
|    `--iterative`     | Build diagramm with iterative algorithm (default:  true)              | -                    |
|   `--large-first`    | Build diagramm with large-first algorithm                             | -                    |
|     `--merging`      | Build diagramm with merging algorithm (not recommended)               | -                    |
|     `-h, --help`     | Print usage                                                           | -                    |

Group representation format example:

```bash
any text before representation
f := FreeGroup( "a", "b" );
g := f / [ a*b*(a)^(-1)*(b)^(-1), a*b*(a)^(-1)*(b)^(-1) ];
any text after representation
```

### Generate [format] file

```bash
dot -T[format] <your-diagramm-path> -o outfile.[format]
```

### Generate svg file

```bash
./generate-svg.sh <file-with-diagram>
```

Supported formats list can be found at [graphviz.org](https://graphviz.org/doc/info/output.html)

## Example

```bash
$ head -c 40 out.txt
local f, g;
f := FreeGroup( "f.1", "f.2"
$ ./vankamp-vis -i out.txt
$ ./generate-svg.sh vankamp-vis-out.dot
```

Will generate Van Kampen diagramm for alphabet group representation in `out.txt` file.

![example](media/example.jpg)
