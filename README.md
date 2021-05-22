# Regex-engine-in-C
## A simple regex engine that supports the following:
1. Greedy and non-greedy
2. The following macros:
   - [] (inclusion only)
     - a-z
     - A-Z
     - 0-9
   - Individual characters and numbers
   - '?'
   - '+'
   - '*'
   - \d
   - \w

## The implementation has the following functions -
1. match()
   Finds the leftmost occurrence of the pattern and functions like re.search(). This is
   the main match function that calls match_here until pattern is found.

2. match_here()
   Matches the pattern at the beginning of the text

3. match_nongreedy()
   Performs non-greedy matching of the pattern in the text ie. until it finds the first 
   occurrence of remaining pattern.
   Also matches 0 or more 

4. match_greedy()
   Performs greedy matching of the pattern in the text ie. until it finds the rightmost
   occurrence of remaining pattern

5. match_class()
   Matches characters in character class at the beginning of the text

6. match_class_char()
   Checks if the given character is present in the character class

7. match_macro()
   Matches \d or \w at the beginning of the text

8. match_plus()
   Matches 1 or more occurrences of the preceding pattern in the text

9. match_question()
   Matches 0 or 1 occurrence of the preceding pattern in the text

10. match_escape()
    Matches escaped characters

11. create_class()
    Creates a struct char_class and updates the values of the elements based on the
    contents of the char class in the pattern
