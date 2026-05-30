import re

with open('main.c', 'r') as f:
    text = f.read()

# We know the bot code is commented out with '//' at the start of each line
# and the receiver code is fully uncommented at the bottom.
# This assumes the commented block is first, and active code is later.
lines = text.split('\n')

bot_lines = []
laptop_lines = []
in_laptop = False

for line in lines:
    if line.startswith('//Receiver Code'):
        in_laptop = True
        continue
    
    if in_laptop:
        laptop_lines.append(line)
    else:
        # Strip exactly one '//' if it exists at start
        if line.startswith('//'):
            bot_lines.append(line[2:])
        else:
            bot_lines.append(line)

new_text = "#ifdef ROLE_BOT\n\n" + "\n".join(bot_lines) + "\n\n#elif defined(ROLE_LAPTOP)\n\n" + "\n".join(laptop_lines) + "\n\n#else\n#error \"Define either ROLE_BOT or ROLE_LAPTOP\"\n#endif\n"

with open('main.c', 'w') as f:
    f.write(new_text)

