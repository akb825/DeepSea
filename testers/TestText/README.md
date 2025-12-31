# Test Text

The Test Text tester (`deepsea_test_text_app` target) tests displaying text. Various cases of different scripts, right to left text, line wrapping, etc. are tested across the various pages.

When running the tester, the left and right arrow keys can be used to cycle through the different test cases. On mobile devices with a touchscreen, a one-finger tap cycles forward and two-finger tap cycles backwards.

## Command-line options

In addition to the common command-line options, the following options are supported:

* `-f/--font <font-path>`: path to the font to use in place of the default for latin text.
* `-l/--low`: use low quality text.
* `-m/--medium`: use medium quality text. This is the default.
* `-H/--high`: use high quality text.
* `-v/--very-high`: use very high quality text.

## Test cases

The following test cases are present. When tessellation shaders are available, there are two copies of the text: one using standard quads and another using a tessellation shader to expand a point into a quad. Apart from the first test case, the tessellation shader text should be identical and aren't included with the example screenshots.

### Basic text with instructions

![Basic text with instructions](doc-images/Instructions.png)

### Emboldened text

![Emboldened text](doc-images/Emboldened.png)

### Forward slanted text

![Forward slanted text](doc-images/SlantedForward.png)

### Backward slanted text

![Backward slanted text](doc-images/SlantedBackward.png)

### Outlined text

![Outlined text](doc-images/Outlined.png)

### Multiple styles with different text ranges

![Multiple styles with different text ranges](doc-images/MultipleStyles.png)

### Different sized text

![Different sized text](doc-images/DifferentSizes.png)

### Different sized text with outlines

![Different sized text with outlines](doc-images/DifferentSizesOutlined.png)

### Different sized text in the same line

![Different sized text in the same line](doc-images/DifferentSizesInLine.png)

### Line wrapping mixed with explicit newlines

![Line wrapping mixed with explicit newlines](doc-images/LineWrapping.png)

### Line wrapping with each word too long

![Line wrapping with each word too long](doc-images/LineWrappingTooLong.png)

### Line wrapping with centered text

![Line wrapping with centered text](doc-images/LineWrappingCentered.png)

### Line wrapping with right-justified text

![Line wrapping with right-justified text](doc-images/LineWrappingRight.png)

### Mixed scripts

![Mixed scripts](doc-images/MixedScripts.png)

### Right-to-left text with punctuation

![Right-to-left text with punctuation](doc-images/RightToLeftPunctuation.png)

### Right-to-left text with line wrapping

![Right-to-left text with line wrapping](doc-images/RightToLeftWrapping.png)

### Right-to-left text with newlines

![Right-to-left text with newlines](doc-images/RightToLeftNewlines.png)

### Line wrapping on a script transition

![Line wrapping on a script transition](doc-images/WrappingScriptTransition.png)

### Right-to-left text at the start and end of the input string

![Right-to-left text at the start and end of the input string](doc-images/RightToLeftOrdering.png)

### Unicode text direction marker

![Unicode text direction marker](doc-images/UnicodeDirectionMarker.png)

### Script transitions with inherited scripts

![Script transitions with inherited scripts](doc-images/InheritedScriptTransitions.png)

### Negative Y offset

![Negative Y offset](doc-images/NegativeYOffset.png)

### Positive Y offset

![Positive Y offset](doc-images/PositiveYOffset.png)

### Uniform text mode with Latin text

![Uniform text mode with Latin text](doc-images/UniformTextLatin.png)

### Uniform text mode with Arabic text

![Uniform text mode with Arabic text](doc-images/UniformTextArabic.png)

### Icons

![Icons](doc-images/Icons.png)
