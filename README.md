toscan
======

A program that scans a text file (presently stdin) for todo items.  Todo
items are paragraphs that begin with the sequence `[ ]`.  This paragraph
would not be detected as a todo item because the sequence is not at the
beginning.  However, the next paragraph would.

	[ ] (EXAMPLE ONLY) This paragraph would be detected as a todo
	    item.  The indentation does not matter.
      Yes it really does not matter!

Things to come...

[ ] Command line flags to tweak what to display.  E.g. completed items
    `[X]`, cancelled `[C]`, not sure items `[?]`
