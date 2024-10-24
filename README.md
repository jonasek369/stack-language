# Stack-based language
## Instructions
- push: adds number to stack [] -> [a]
- pop: removes number from stack [a] -> []
- add: adds two numbers on stack [a, b] -> [a+b]
- sub: subtracts two numbers on stack [a, b] -> [a-b]
- cmp: compares two numbers, pushes 1 or 0 on stack [a, b] -> [a==b]
- store [var_name]: stores number into heap [a] -> [] (a is now in heap)
- load [var_name]: loads number into stack from heap [] -> [a] (value taken from heap)
- loadf [var_name]: loads number into stack from heap and frees the memory. So [var_name] doesn't exist now [] -> [a]
- jmp: jumps to label (also can jump to a number, but it is token count, not line count). Doesn't change stack, just instructionPointer
- jnz: pops value from stack; if it's not 0, jumps to label (or number, same as jmp). Same as jmp
- inc: pop value, add 1, and push back n+1 [a] -> [a+1]
- dec: pop value, subtract 1, and push back n-1 [a] -> [a-1]
- switch: switches the top value and the value under it [a, b] -> [b, a]
- not: pop value, apply logical not to the value, and push onto stack (0 -> 1, n -> 0) [a] -> [!a]
- label [label_name]: creates label with [label_name] that can be used for jumps
- rot: rotates the stack [a, b, c] -> [c, b, a]
- dup: duplicates top value on stack [a] -> [a, a]
- over: duplicates value behind the top [a, b] -> [a, b, a]
- out: pops value and prints it out as a number
- outc: pops value and prints it out as a character
