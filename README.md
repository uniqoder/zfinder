# zfinder: A Fuzzy File Finder

`zfinder` is a command-line tool for searching files and directories recursively within a specified base directory.

It supports filtering by name, extension, size, and item type. Name filter uses fuzzy matching; the other filters use exact matching.

![zfinder demo](./docs/demo.gif)

---

## How It Works

`zfinder` recursively searches a specified base directory for files and directories that match the supplied filters.

- The maximum recursion depth can be configured through command-line arguments.
- When a name or extension filter is used, colors indicate which part of each item's name matched.
- Results are streamed without any order but the top ranking results are shown at the end (with a special flag).
- Every item must satisfy each filter to succeed.

---

## Usage

`zfinder` is a command-line program. You provide filters and configuration options through command-line arguments.

The following arguments are implemented for `zfinder`:

| Long option | Short option | Input type | Example | Description |
|---|---|---|---|---|
| `--name` | `-n` | String | `--name "log"` | Fuzzily matches an item's name. |
| `--type` | `-t` | Character | `--type f` | Matches an item's type exactly. Supported values: `f` = file, `d` = directory, `l` = symbolic link. |
| `--extension` | `-e` | String | `--extension "txt"` | Matches an item's extension exactly. An empty string matches items without an extension. |
| `--size` | `-s` | Size | `--size "<100M"` | Matches an item's size exactly. |
| `--max-depth` | `-d` | Number | `--max-depth 2` | Limits the maximum recursion depth. Depth 0 is the base directory. |
| `--dir-size` | `-ds` | None | `-ds` | Calculates size of directories based on their elements. |
| `--show-best` | `-sb` | None | `--show-best` | Lists the best matches at the end. Toggles automatically when Name filter is used. |
| `--help` | `-h` | None | `-h` | Prints some helpful text... |

The exact command looks like this:
```bash
zf [base path] [filters]
```

### Important Notes

#### Links

Symbolic links to directories are not followed during the search.

#### Extension

The extension filter just checks if the item name ends with that suffix. This means that you can provide longer extensions like `bak.old.tar.gz`.
This also means that the filter `name` would match `./src/test/filename` exactly.

#### Size

The size filter takes a complex input.
Size of directories is calculated based on its children, the directory's own logical size is not included. Empty directories have size = `0`.
Following are the types of input to the size argument:

| Name | Form | Example |
|---|---|---|
| Exact match | s | `--size "2G"` |
| Less than | <s | `--size "<25M"` |
| Less or equal | <=s | `--size "<=980"` |
| Greater than | >s | `--size ">1m"` |
| Greater or equal | >=s | `--size ">=89"` |
| Between | l..m | `--size "23M..1g"` |

For range, the lower limit is inclusive, while the upper limit is exclusive.

#### Size suffixes

Sizes are specified as numbers with an optional suffix. Sizes are measured in bytes, not bits.

| Suffix | Multiplier |
|---|---:|
| `K` | \(10^3\) |
| `M` | \(10^6\) |
| `G` | \(10^9\) |
| `T` | \(10^{12}\) |
| `k` | \(2^{10}\) |
| `m` | \(2^{20}\) |
| `g` | \(2^{30}\) |
| `t` | \(2^{40}\) |


#### Fuzzy matching

Fuzzy name matching uses Levenshtein's Edit Distance algorithm to find the distance between the query and target.
Then, this distance is normalized by dividing it by the max length between the query and target.
Accuracy reduces when target and query contain different number of parts (separated at '.').
Target string containing the query as a sub-string increases accuracy.

### Examples

```bash
zf ./logs --name "test" --type f --size "<=50K"
zf src -e "cpp" -t f
zf . -name "tests" -t d
zf foo/bar --size "24K..1M"
zf ~/.local/bin --type l --max-depth 0 -nc | sort
```

---

## Installation

You can build `zfinder` from source by cloning the repository and compiling it, or download a prebuilt binary from the project's [releases page](https://github.com/uniqoder/zfinder/releases).

### Building from Source

Clone the repository:

```bash
git clone https://github.com/uniqoder/zfinder.git
cd zfinder
```

Compile the project:

```bash
make
```

You can then run the binary from the `build` directory. To install it so that it can be run from anywhere, use:

```bash
sudo make install
```

If you need to uninstall it, just go to the repo directory and run:

```bash
sudo make uninstall
```
