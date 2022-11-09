# PrismFileFormat
Low level File format access for Samuel Harmon's PrismScroll .prism files

## What's this all about?
I find rolling random treasure for D&D 5e campaigns to be a chore. The tables in the DMG are clunky 
and don't contain new content in later core rule books, not to mention adventures. So, it seemed
like a iPad app or similar might be nice.  Such things are pretty easy to write. The problem is
the database of extant magic items.  We use the FightClub 5e app for much of our D&D games, and it
has one such database associated with it here on github that it can import. Alas, the database doesn't
include the "rarity" tag, which is rather important for a random treasure generator. 

I recently found Samuel Harmon's PrismScroll and PrismScrollDM apps. These are in active development
and have a similar database hiding off in a discord channel. He also keeps a .prism file for the SRD
with the app in the Resources subdirectory.  Alas, it isn't quite XML and without newlines was rather
painful to look at in depth, but was simple enough in structure to reverse engineer in a few hours.
Now, I can read it. It has a "rarity" tag. Yay!

## What this software does
The attached code (compatible with PrismScroll 6.22.0 to my limited testing) can read the .prism file and produce
a hierarchy of C++ objects corresponding to sets, arrays, key-value pairs, strings, bools, ints and 
doubles. It can print them out in a more human readable form. It is also capable of taking such trees 
and writing a .prism file that diffs without loss against the original, at least for the two .prism 
files I looked at, the included SRD file in the app and the larger basic one from Discord. (It needed testing.) 

## License
This is available under the MIT license (Open Source Initiative). 

Copyright 2022, Ian Ollmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This software does not contain code by Samuel Harmon or PrismScroll, nor is it endorsed or maintained by him in any way. 
This work contains no copyrighted material belonging to Wizards of the Coast(TM) or Hasbro(TM).
