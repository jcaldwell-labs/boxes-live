# Boxes-Live Acceptance Tests
# Gherkin/BDD specifications for boxes-live 1.0.0

Feature: Canvas Creation and Management
  As a user
  I want to create and manage canvas workspaces
  So that I can organize visual information

  Scenario: Create a new canvas
    Given boxes-live is installed
    When I run "boxes-live"
    Then the application should start successfully
    And I should see an empty canvas
    And I should see the status bar with controls

  Scenario: Load an existing canvas
    Given a canvas file "test_canvas.txt" exists
    When I press "F3"
    And I enter "test_canvas.txt"
    Then the canvas should load successfully
    And all boxes from the file should be visible
    And the viewport should be at the saved camera position

  Scenario: Save current canvas
    Given I have a canvas with 3 boxes
    When I press "F2"
    And I enter "my_canvas.txt"
    Then the file "my_canvas.txt" should be created
    And the file should contain all 3 boxes
    And the file should be in BOXES_CANVAS_V1 format

Feature: Viewport Navigation
  As a user
  I want to navigate around the canvas
  So that I can explore large workspaces

  Scenario: Pan the viewport with arrow keys
    Given I have a canvas with boxes at various positions
    When I press "Right Arrow" 10 times
    Then the camera should move 100 pixels right
    And previously visible boxes may scroll out of view
    And the status bar should show updated camera position

  Scenario: Pan with WASD keys
    Given I have a canvas loaded
    When I press "W" key
    Then the camera should move up
    When I press "A" key
    Then the camera should move left
    When I press "S" key
    Then the camera should move down
    When I press "D" key
    Then the camera should move right

  Scenario: Zoom in with +
    Given I have a canvas with visible boxes
    When I press "+" key
    Then the zoom level should increase by 1.2x
    And boxes should appear larger
    And the zoom should center on viewport center

  Scenario: Zoom out with -
    Given I have a canvas with visible boxes
    When I press "-" key
    Then the zoom level should decrease by 1/1.2x
    And boxes should appear smaller
    And more boxes should be visible

  Scenario: Reset view with R
    Given I have panned and zoomed the viewport
    When I press "R" or "0"
    Then the camera should return to (0, 0)
    And the zoom should reset to 1.0x

Feature: Box Selection with Mouse
  As a user
  I want to select boxes by clicking them
  So that I can interact with specific boxes

  Scenario: Click to select a box
    Given I have a canvas with 3 boxes
    When I click on box #2
    Then box #2 should be selected
    And box #2 should be highlighted
    And previously selected boxes should be deselected

  Scenario: Click empty space deselects
    Given box #1 is currently selected
    When I click on empty canvas space
    Then no boxes should be selected
    And the highlight should disappear

Feature: Box Dragging
  As a user
  I want to drag boxes to reposition them
  So that I can organize my workspace

  Scenario: Drag a box to new position
    Given I have a box at position (100, 100)
    When I click and hold on the box
    And I drag the mouse to position (200, 150)
    And I release the mouse button
    Then the box should move to (200, 150)
    And the new position should persist when saved

  Scenario: Drag updates in real-time
    Given I have selected a box
    When I start dragging
    Then the box should follow the mouse cursor
    And the box should update position smoothly

Feature: Box Colors
  As a user
  I want boxes to have different colors
  So that I can categorize information visually

  Scenario: Boxes display with assigned colors
    Given I have boxes with different color values
    When I view the canvas
    Then box with color 1 should be red
    And box with color 2 should be green
    And box with color 3 should be blue
    And box with color 4 should be yellow
    And box with color 5 should be magenta
    And box with color 6 should be cyan
    And box with color 7 should be white

Feature: CLI Tools - Canvas Creation
  As a developer or power user
  I want to create canvases programmatically
  So that I can automate canvas generation

  Scenario: Create new canvas with boxes-cli
    Given boxes-cli is executable
    When I run "boxes-cli create my_canvas.txt"
    Then the file "my_canvas.txt" should be created
    And the file should contain an empty canvas
    And the canvas should have default dimensions 2000x1500

  Scenario: Create canvas with custom dimensions
    Given boxes-cli is available
    When I run "boxes-cli create test.txt --width 3000 --height 2000"
    Then the canvas should be 3000x2000 pixels
    And the canvas file should be valid

Feature: CLI Tools - Box Management
  As a developer
  I want to add, update, and delete boxes via CLI
  So that I can script canvas manipulation

  Scenario: Add a box to canvas
    Given a canvas file "test.txt" exists
    When I run:
      """
      boxes-cli add test.txt \
        --x 100 --y 100 \
        --title "My Box" \
        --content "Line 1" --content "Line 2" \
        --color 2
      """
    Then the canvas should contain a new box
    And the box should have title "My Box"
    And the box should be at position (100, 100)
    And the box should have 2 content lines
    And the box should be green (color 2)

  Scenario: Update existing box
    Given a canvas with box #1
    When I run "boxes-cli update test.txt 1 --title 'Updated Title'"
    Then box #1 should have title "Updated Title"
    And other box properties should be unchanged

  Scenario: Delete a box
    Given a canvas with 3 boxes
    When I run "boxes-cli delete test.txt 2"
    Then the canvas should have 2 boxes
    And box #2 should be removed

  Scenario: List all boxes
    Given a canvas with 5 boxes
    When I run "boxes-cli list test.txt"
    Then I should see 5 boxes listed
    And each box should show: ID, title, position, color

Feature: CLI Tools - Search and Query
  As a user
  I want to search and filter boxes
  So that I can find relevant information

  Scenario: Search boxes by keyword
    Given a canvas with boxes containing various text
    When I run "boxes-cli search test.txt 'database'"
    Then I should see only boxes containing "database"
    And each result should show box ID and title

  Scenario: Get box statistics
    Given a canvas with boxes of different colors
    When I run "boxes-cli stats test.txt"
    Then I should see total box count
    And I should see canvas dimensions
    And I should see color distribution
    And I should see average lines per box

Feature: CLI Tools - Export and Import
  As a user
  I want to export canvases to different formats
  So that I can share and process canvas data

  Scenario: Export to Markdown
    Given a canvas with 3 boxes
    When I run "boxes-cli export test.txt --format markdown -o output.md"
    Then "output.md" should be created
    And the file should be valid Markdown
    And each box should be a section with title and content

  Scenario: Export to JSON
    Given a canvas file "test.txt"
    When I run "boxes-cli export test.txt --format json -o output.json"
    Then "output.json" should be valid JSON
    And the JSON should contain all box properties
    And the JSON should preserve box positions and colors

  Scenario: Export to CSV
    Given a canvas with boxes
    When I run "boxes-cli export test.txt --format csv -o output.csv"
    Then "output.csv" should be valid CSV
    And each row should represent one box
    And columns should include: ID, title, x, y, color, content

Feature: Connectors - Git History
  As a developer
  I want to visualize git history as boxes
  So that I can explore commits spatially

  Scenario: Generate git timeline
    Given I am in a git repository
    When I run "git2canvas --max 20"
    Then I should see 20 commit boxes
    And boxes should be arranged chronologically
    And each box should show: hash, author, date, message
    And boxes should be color-coded by author

  Scenario: Filter by branch
    Given I am in a git repository with multiple branches
    When I run "git2canvas --branch feature/foo"
    Then I should see only commits from feature/foo branch

Feature: Connectors - Directory Tree
  As a user
  I want to visualize directory structures
  So that I can understand project organization

  Scenario: Generate directory tree canvas
    Given I am in a project directory
    When I run "tree2canvas . --max-depth 3"
    Then I should see directory boxes arranged hierarchically
    And subdirectories should be positioned to the right
    And boxes should show directory names and file counts

  Scenario: Include files in tree
    Given I am in a directory
    When I run "tree2canvas . --show-files"
    Then I should see both directory and file boxes
    And files should be visually distinct from directories

Feature: Connectors - CSV Import
  As a data analyst
  I want to convert CSV data to visual canvas
  So that I can explore data spatially

  Scenario: Import CSV with default settings
    Given a CSV file "data.csv" with headers
    When I run "csv2canvas data.csv"
    Then each CSV row should become a box
    And the first column should be the box title
    And all columns should appear in box content
    And boxes should be arranged in grid layout

  Scenario: Custom title and color columns
    Given a CSV file with Name, Role, Priority columns
    When I run:
      """
      csv2canvas data.csv \
        --title-col 0 \
        --color-col 2 \
        --layout grid
      """
    Then each box title should be from Name column
    And box colors should map from Priority column

Feature: Connectors - Text to Boxes
  As a writer
  I want to convert text documents to boxes
  So that I can rearrange and organize content

  Scenario: Split text by lines
    Given a text file with 10 lines
    When I run "text2canvas notes.txt --lines"
    Then I should see 10 boxes
    And each box should contain one line

  Scenario: Split by paragraphs
    Given a document with 5 paragraphs
    When I run "text2canvas doc.txt --paras"
    Then I should see 5 boxes
    And each box should contain one paragraph

  Scenario: Split by markdown sections
    Given a Markdown file with ## headers
    When I run "text2canvas README.md --sections"
    Then each section should become a box
    And box titles should be section headers

Feature: Signal Handling
  As a user
  I want the application to handle signals gracefully
  So that my data is not corrupted on exit

  Scenario: SIGINT (Ctrl+C) cleanup
    Given boxes-live is running
    When I press Ctrl+C
    Then the application should cleanup ncurses
    And the terminal should be restored
    And no core dump should occur

  Scenario: SIGTERM cleanup
    Given boxes-live is running with PID 12345
    When I send SIGTERM to the process
    Then the application should exit gracefully
    And resources should be freed

Feature: Performance and Scalability
  As a user
  I want the application to perform well
  So that I can work with large canvases smoothly

  Scenario: Handle large canvases
    Given a canvas with 1000 boxes
    When I load and render the canvas
    Then the application should remain responsive
    And frame rate should be >= 30 FPS
    And pan/zoom should be smooth

  Scenario: Viewport culling optimization
    Given a canvas with 500 boxes
    And only 20 boxes are visible in viewport
    When I render the canvas
    Then only visible boxes should be drawn
    And invisible boxes should be culled
    And rendering should be fast

Feature: Error Handling
  As a user
  I want clear error messages
  So that I can understand and fix problems

  Scenario: Load non-existent file
    Given the file "missing.txt" does not exist
    When I try to load "missing.txt" with F3
    Then I should see an error message
    And the current canvas should remain unchanged
    And the application should not crash

  Scenario: Load malformed canvas file
    Given a file "bad.txt" with invalid format
    When I try to load "bad.txt"
    Then I should see "Invalid canvas format" error
    And the application should remain stable

  Scenario: CLI with invalid arguments
    Given boxes-cli is available
    When I run "boxes-cli add canvas.txt --invalid-flag"
    Then I should see usage help
    And the exit code should be non-zero

Feature: Accessibility and Usability
  As a user
  I want helpful feedback
  So that I can learn and use the application effectively

  Scenario: Status bar shows controls
    Given boxes-live is running
    When I view the status bar
    Then I should see "F2:Save F3:Load Q:Quit"
    And I should see camera position
    And I should see zoom level

  Scenario: Help command shows usage
    Given boxes-live is available
    When I run "boxes-live --help"
    Then I should see usage instructions
    And I should see all command-line options
    And I should see keyboard controls

Feature: Integration - Unix Pipeline Composition
  As a Unix user
  I want to compose tools with pipes
  So that I can build complex workflows

  Scenario: Chain connectors and CLI
    Given I have git2canvas and boxes-cli
    When I run:
      """
      git2canvas --max 10 | \
        boxes-cli arrange --layout grid | \
        boxes-cli export --format markdown
      """
    Then I should see Markdown output
    And the output should contain 10 git commits
    And commits should be formatted as Markdown sections

  Scenario: CSV transformation pipeline
    Given a CSV file "data.csv"
    When I run:
      """
      csv2canvas data.csv | \
        boxes-cli search 'priority:high' | \
        boxes-cli export --format json
      """
    Then I should see JSON with high-priority items only
