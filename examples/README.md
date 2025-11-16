# boxes-live Examples and Demonstrations

This directory contains example canvases and demonstrations of boxes-live connectors.

## Quick Start Examples

### 1. Git History Visualization

View your git commit history as an interactive timeline:

```bash
# Generate git history canvas
./connectors/git2canvas --max 20 > examples/my_commits.txt

# View interactively
boxes-live  # Press F3, load examples/my_commits.txt

# Or use CLI to explore
./connectors/boxes-cli list examples/my_commits.txt
./connectors/boxes-cli search examples/my_commits.txt "feature"
```

**Features**:
- Color-coded by author
- Horizontal timeline layout
- Shows commit hash, author, date, message
- Filter by author, branch, date range

### 2. Directory Structure Explorer

Visualize your project structure:

```bash
# Visualize current directory
./connectors/tree2canvas . --max-depth 3 > examples/project_tree.txt

# Include files, not just directories
./connectors/tree2canvas . --max-depth 2 --show-files > examples/full_tree.txt

# View interactively
boxes-live  # F3 → examples/project_tree.txt
```

**Features**:
- Color-coded by directory depth
- Hierarchical left-to-right layout
- Shows item counts for directories
- Auto-excludes .git, node_modules, __pycache__

### 3. CSV Data Visualization

Turn spreadsheet data into an interactive canvas:

```bash
# Convert CSV to canvas
./connectors/csv2canvas data.csv > examples/data_canvas.txt

# With custom title and color columns
./connectors/csv2canvas data.csv \
    --title-col 0 \
    --color-col 2 \
    --layout grid > examples/data_canvas.txt

# Export back to different formats
./connectors/boxes-cli export examples/data_canvas.txt \
    --format markdown -o report.md
```

**Features**:
- Grid or flow layout
- Color mapping by column values
- All columns visible in box content
- Round-trip capable (CSV → canvas → export)

### 4. Text File to Boxes

Convert any text file to a navigable canvas:

```bash
# Each line becomes a box
./connectors/text2canvas notes.txt --lines > examples/notes.txt

# Each paragraph becomes a box
./connectors/text2canvas document.txt --paras > examples/document.txt

# Split on markdown headers
./connectors/text2canvas README.md --sections > examples/readme_sections.txt
```

**Features**:
- Multiple split modes (lines, paragraphs, sections)
- Auto-layout in grid or flow
- Preserves content structure

### 5. Project Planning (CLI Demo)

See a complete project planning workflow:

```bash
# Run the demo
./examples/cli_demo.sh

# Creates project canvas with phases, tasks, and exports
# Outputs:
#   - examples/project_canvas.txt (interactive canvas)
#   - examples/project.md (Markdown export)
#   - examples/project.json (JSON export)
#   - examples/project.csv (CSV export)
```

## Working with Generated Canvases

### View Interactively

```bash
# Launch boxes-live
boxes-live

# Inside boxes-live:
# - Press F3 to open file
# - Type the canvas filename
# - Navigate with arrow keys / WASD
# - Zoom with +/- or Z/X
# - Click boxes to select
# - Drag boxes to reposition
# - Press F2 to save changes
```

### Export to Different Formats

```bash
# Markdown report
./connectors/boxes-cli export canvas.txt --format markdown -o report.md

# JSON for programmatic access
./connectors/boxes-cli export canvas.txt --format json -o data.json

# CSV for spreadsheets
./connectors/boxes-cli export canvas.txt --format csv -o spreadsheet.csv
```

### Query and Filter

```bash
# List all boxes
./connectors/boxes-cli list canvas.txt

# Search by keyword
./connectors/boxes-cli search canvas.txt "feature"

# Filter by color
./connectors/boxes-cli list canvas.txt --color 3

# Get statistics
./connectors/boxes-cli stats canvas.txt
```

## Workflow Examples

### Developer Workflow: Code Exploration

```bash
# 1. Visualize repository structure
./connectors/tree2canvas . --max-depth 3 > codebase.txt

# 2. View git history
./connectors/git2canvas --max 30 --since "2024-01-01" > git_history.txt

# 3. Explore interactively
boxes-live  # Load and navigate both canvases

# 4. Export findings
./connectors/boxes-cli export codebase.txt --format markdown -o structure.md
```

### Data Analysis Workflow

```bash
# 1. Import CSV data
./connectors/csv2canvas sales_data.csv --title-col 1 --color-col 3 > sales.txt

# 2. Arrange for better visualization
./connectors/boxes-cli arrange sales.txt --layout grid

# 3. Add annotations by editing in boxes-live
boxes-live  # F3 → sales.txt, add notes, F2 to save

# 4. Export annotated analysis
./connectors/boxes-cli export sales.txt --format markdown -o analysis.md
```

### Documentation Workflow

```bash
# 1. Convert markdown docs to navigable canvas
./connectors/text2canvas DESIGN.md --sections > design_canvas.txt

# 2. Rearrange sections spatially in boxes-live

# 3. Add cross-references and notes

# 4. Export back to linear markdown
./connectors/boxes-cli export design_canvas.txt --format markdown -o DESIGN_v2.md
```

## Available Example Files

After running connectors and demos, you'll find:

- `git_history.txt` - Git commit visualization
- `directory_tree.txt` - Project structure
- `project_canvas.txt` - Project planning example
- `project_data.txt` - CSV data visualization
- `project.md` - Markdown export example
- `project.json` - JSON export example
- `project.csv` - CSV export example
- `sample_data.csv` - Sample CSV data

## Creating Custom Workflows

Chain connectors with Unix pipes:

```bash
# Combine operations
cat data.csv | ./connectors/csv2canvas | \
    ./connectors/boxes-cli arrange --layout grid > organized.txt

# Filter and transform
./connectors/git2canvas --author "John" --max 50 | \
    ./connectors/boxes-cli export --format json > john_commits.json

# Complex pipeline
find . -name "*.py" -type f | \
    head -20 | \
    xargs -I {} echo {} | \
    ./connectors/text2canvas --lines > python_files.txt
```

## Tips and Tricks

### Performance

- Use `--max-depth` with tree2canvas for large directories
- Limit `--max` commits with git2canvas for faster rendering
- Use `--exclude` patterns to skip irrelevant files

### Layout

- Grid layout: Best for data with similar items
- Flow layout: Better for varying box sizes
- Manual arrangement: Load in boxes-live, drag to organize, save

### Colors

- Colors group related items visually
- Use `--color-col` to map data values to colors
- 7 colors available (red, green, blue, yellow, magenta, cyan, white)

### Export and Share

- Markdown: Human-readable reports
- JSON: Programmatic processing
- CSV: Import to Excel/Google Sheets
- Canvas .txt: Share for interactive exploration

## See Also

- [connectors/README.md](../connectors/README.md) - Complete CLI documentation
- [CONNECTORS.md](../CONNECTORS.md) - Connector design patterns
- [PLUGIN_ARCHITECTURE.md](../PLUGIN_ARCHITECTURE.md) - Live-updating plugins

## Need Help?

```bash
# Connector help
./connectors/git2canvas --help
./connectors/csv2canvas --help
./connectors/tree2canvas --help

# CLI help
./connectors/boxes-cli --help
./connectors/boxes-cli <command> --help

# Interactive help
boxes-live  # Press '?' for help (if implemented)
```
