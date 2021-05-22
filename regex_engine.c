#include <stdio.h>
#include <stdlib.h>

typedef struct node{
	int lower;	// 1 if a-z included
	int upper;	// 1 if A-Z included
	int d;	// 1 if 0-9 included
	int chars_up[26];	// chars[i] will be 1 if 'a' + i is included in char class
	int chars_lo[26];
	int digits[10];	// digits[i] will be 1 if i is included in the char class
	char other[10];	// match any other chars
} char_class;

#define DEBUG 0
int match_here(char *pat, char *text, int* i);
int match_non_greedy(char ch, char* pat, char* text, int* i);
int match_greedy(char ch, char* pat, char* text, int* i);
int match_class(char* pat, char* text, int* i);
int match_class_char(char* text, char_class* class);
int match_macro(char* pat, char* text, int* i);
int match_plus(char c, char *regexp, char *text, int* i);
int match_question(char c, char* pat, char* text, int* i);
int match_escape(char* pat, char* text, int* i);
char_class* create_class(char* pat, int* end);

// main match function that finds a match, if any, anywhere in the text like search
int match(char *pat, char *text)
{
	int i = 0;
	// find pattern at the beginning of the text
	if(*pat == '^')
	{
		if(match_here(pat + 1, text, &i)){
			printf("1 %d %d\n", 0, i-1);
			return 1;
		}
	}
	// find leftmost match
	do
	{
		int temp = i;
		if(match_here(pat, text, &temp)){
			printf("1 %d %d\n", i, (temp > 0) ? temp - 1 : 0);
			return 1;
		}
		i++;
	} while(*text++ != '\0');
	printf("0\n");
	return 0;
}

// matches the pattern at the beginning of the given text
int match_here(char *pat, char *text, int* i)
{
	// empty pattern
	if((*pat) == '\0')
		return 1;
	if(pat[1] == '*' && (pat[2] == '?') && (pat[0] != '\\'))
		return match_non_greedy(pat[0], pat+3, text, i);
	if(pat[1] == '*' && (pat[0] != '.') && (pat[0] != '\\'))
		return match_non_greedy(pat[0], pat+2, text, i);
	if(pat[1] == '*' && (pat[0] == '.') && (pat[0] != '\\'))
		return match_greedy(pat[0], pat+2, text, i);
	if(pat[0] == '$' && pat[1] == '\0')
		return (*text == '\0');	
	if((pat[1] == '+') && (pat[2] == '?') && (pat[0] != '\\'))
		return match_plus(pat[0], pat+3, text, i);
	if(pat[1] == '+' && (pat[0] != '\\'))
		return match_plus(pat[0], pat+2, text, i);	
	if(pat[1] == '?' && (pat[0] != '\\'))
		return match_question(pat[0], pat+2, text, i);
	if(pat[0] == '[')
		return match_class(pat+1, text, i);
	if(pat[0] == '\\' && ((pat[1] == 'w') || (pat[1] == 'd')))
		return match_macro(pat+1, text, i);
	if((pat[0] == '\\') && (pat[1] != 'd') && (pat[1] != 'w'))
		return match_escape(pat+1, text, i);
	if((*text != '\0') && ((*pat == *text) || (*pat == '.')))
	{
		(*i)++;
		return match_here(pat + 1, text + 1, i);
	}
	return 0;	
}

// returns 1 if the current text char doesn't match with the input char
int mismatch(char ch, char* text){
	if(*text == '\0')
		return 1;
	if(ch == '.')
		return 0;
	if(*text != ch)
		return 1;
	return 0;
}

// performs a non-greedy match of pattern and text
int match_non_greedy(char c, char *regexp, char *text, int* i)
{
	//a* matches zero or more instances
    do {    
		int temp = *i;
		// terminate matching once non-matching pair of chars found and resume matching from next char of pattern and text
        if (mismatch(c, text) || ((c == '.') ? match_here(regexp,text,i) : 0) || (*regexp == '\0')){
        	if((c == '.') && (!mismatch(c,text))){
        		(*i) = temp;
        	}
            return match_here(regexp,text,i);
        }
        (*i) = temp;
    } while (*text != '\0' && (*text++ == c || c == '.') && (++(*i)));
    return 0;
}

int match_macro(char* pat, char* text, int* i){
	char_class* class = (char_class*)malloc(sizeof(char_class)); // create a char class
	
	// initialize struct values
	class->lower = 0;
	class->upper = 0;
	class->d = 0;
	for (int i = 0; i < 26; ++i)
	{
		class->chars_up[i] = 0;
		class->chars_lo[i] = 0;
	}
	for (int i = 0; i < 10; ++i)
	{
		class->digits[i] = 0;
		class->other[i] = '\n';
	}

	// update struct values according to the macro
	if(*pat == 'd')
		class->d = 1;
	if(*pat == 'w'){
		class->lower = 1;
		class->upper = 1;
		class->d = 1;
		class->other[0] = '_';
	}

	char ch = *(pat + 1);
	if(ch == '*' && (*(pat + 2) == '?')){	// non-greedy match of char class
	    do {   
			int temp = *i;
	        if (!(match_class_char(text, class)) || match_here(pat+3,text,i) || (*(pat+3) == '\0')){
	        	if(match_class_char(text,class))
	        		(*i) = temp;
	        	free(class);
	            return match_here(pat+3,text,i);
	        }
	        (*i) = temp;
	    } while (*text != '\0' && (match_class_char(text++, class)) && (++(*i)));
	    free(class);
	    return 0;		
	}
	else if(ch == '*'){	// 0 or more occurrences of char class
		char *t;
		for (t = text; *t != '\0' && (match_class_char(t, class)); t++, (*i)++)
			;
		do {
			int temp = *i;
			if (match_here(pat+2, t, i)){
				free(class);
				return 1;
			}
			*i = temp;
		} while ((t-- > text) && (*i)--);
		free(class);
		return 0;
	}
	else if(ch == '+'){		// 1 or more occurrences of char class
		if(*(pat+2) == '?'){	
		    while (*text != '\0' && (match_class_char(text++, class)) && (++(*i))) {    
				int temp = *i;
				// terminate matching once non-matching pair of chars found and resume matching from next char of pattern and text
		        if (!(match_class_char(text, class)) || match_here(pat+3,text,i) || (*(pat+3) == '\0')){
		        	if(!(match_class_char(text, class))){
		        		(*i) = temp;
		        	}
		        	free(class);
		            return match_here(pat+3,text,i);
		        }
		        (*i) = temp;
		    }
		    free(class);
		    return 0;
		}
	    while (*text != '\0' && (match_class_char(text++, class)))
	    {    
	    	(++(*i));
			int temp = *i;
	        if (!(match_class_char(text, class)) || ((*(pat+2) != '\0') ? match_here(pat+2,text,i) : 0)){
	        	if(match_class_char(text, class))
	        		(*i) = temp;
	        	free(class);
	            return match_here(pat+2,text,i);
	        }
	        (*i) = temp;        
	    } 
	    free(class);
	    return 0;

	}
	else if(ch == '?'){		// 0 or 1 occurrence of char clas
	    if (*text != '\0' && (match_class_char(text, class)))
	    {    
	    	(++(*i)); 
	    	text++;   
	    } 
	    free(class);
		return match_here(pat+2, text, i);
	}
	else{	// match single char from char class
		if(match_class_char(text, class)){
			(*i)++;
			free(class);
			return match_here(pat + 1, text + 1, i);
		} 
		free(class);
		return 0;
	}
}

int match_escape(char* pat, char* text, int* i){
	char_class* class = (char_class*)malloc(sizeof(char_class)); // create a char class
	
	// initialize struct values
	class->lower = 0;
	class->upper = 0;
	class->d = 0;
	for (int i = 0; i < 26; ++i)
	{
		class->chars_up[i] = 0;
		class->chars_lo[i] = 0;
	}
	for (int i = 0; i < 10; ++i)
	{
		class->digits[i] = 0;
		class->other[i] = '\n';
	}

	// update struct values accordingly
	class->other[0] = *pat;

	char ch = *(pat + 1);
	if(ch == '*' && (*(pat + 2) == '?')){	// non-greedy match of char class
	    do {   
			int temp = *i;
	        if (!(match_class_char(text, class)) || match_here(pat+3,text,i) || (*(pat+3) == '\0')){
	        	if(match_class_char(text,class))
	        		(*i) = temp;
	        	free(class);
	            return match_here(pat+3,text,i);
	        }
	        (*i) = temp;
	    } while (*text != '\0' && (match_class_char(text++, class)) && (++(*i)));
	    free(class);
	    return 0;		
	}
	else if(ch == '*'){	// 0 or more occurrences of char class
		char *t;
		for (t = text; *t != '\0' && (match_class_char(t, class)); t++, (*i)++)
			;
		do {
			int temp = *i;
			if (match_here(pat+2, t, i)){
				free(class);
				return 1;
			}
			*i = temp;
		} while ((t-- > text) && (*i)--);
		free(class);
		return 0;
	}
	else if(ch == '+'){		// 1 or more occurrences of char class
		if(*(pat+2) == '?'){	
		    while (*text != '\0' && (match_class_char(text++, class)) && (++(*i))) {    
				int temp = *i;
				// terminate matching once non-matching pair of chars found and resume matching from next char of pattern and text
		        if (!(match_class_char(text, class)) || match_here(pat+3,text,i) || (*(pat+3) == '\0')){
		        	if(!(match_class_char(text, class))){
		        		(*i) = temp;
		        	}
		        	free(class);
		            return match_here(pat+3,text,i);
		        }
		        (*i) = temp;
		    }
		    free(class);
		    return 0;
		}
	    while (*text != '\0' && (match_class_char(text++, class)))
	    {    
	    	(++(*i));
			int temp = *i;
	        if (!(match_class_char(text, class)) || ((*(pat+2) != '\0') ? match_here(pat+2,text,i) : 0)){
	        	if(match_class_char(text, class))
	        		(*i) = temp;
	        	free(class);
	            return match_here(pat+2,text,i);
	        }
	        (*i) = temp;        
	    }
	    free(class); 
	    return 0;

	}
	else if(ch == '?'){		// 0 or 1 occurrence of char clas
	    if (*text != '\0' && (match_class_char(text, class)))
	    {    /* a * matches zero or more instances */
	    	(++(*i)); 
	    	text++;   
	    }
	    free(class); 
		return match_here(pat+2, text, i);
	}
	else{		// match single char from char class
		if(match_class_char(text, class)){
			(*i)++;
			free(class);
			return match_here(pat + 1, text + 1, i);
		} 
		free(class);
		return 0;
	}
}

// performs greedy matching of pattern against text
int match_greedy(char c, char *regexp, char *text, int* i)
{
	// find the rightmost match
	char *t;
	for (t = text; *t != '\0' && (*t == c || c == '.'); t++, (*i)++)	// match greedily as many chars as possible
		;

	// backtrack
	do {	
		int temp = *i;
		if (match_here(regexp, t, i))
			return 1;
		else
			(*i) = temp;
	} while ((t-- > text) && (*i)--);
	return 0;
}

// performs matching of 1 or more occurrences of pattern char
int match_plus(char c, char *regexp, char *text, int* i)
{
	if(*(regexp) == '?'){	
		regexp++;
	    while (*text != '\0' && (*text++ == c || c == '.') && (++(*i))) {    
			int temp = *i;
			// terminate matching once non-matching pair of chars found and resume matching from next char of pattern and text
	        if (mismatch(c, text) || match_here(regexp,text,i) || (*regexp == '\0')){
	        	if(!mismatch(c,text)){
	        		(*i) = temp;
	        	}
	            return match_here(regexp,text,i);
	        }
	        (*i) = temp;
	    }
	    return 0;
	}
	else{
		if(c == (*text) || (c == '.')){
			++(*i);
			return match_greedy(c,regexp,text+1,i);
		}
		return 0;
	}
}

// performs matching of 0 or 1 occurrences of pattern char
int match_question(char c, char* pat, char* text, int* i){
    if (*text != '\0' && (*text == c || c == '.'))
    {    
    	(++(*i)); 
    	text++;   
    } 
	return match_here(pat, text, i);	
}

// creates a char class
char_class* create_class(char* pat, int* end){
	// create and initialize values of struct attributes
	char* temp = pat;
	char_class* new = (char_class*)malloc(sizeof(char_class));
	new->lower = 0;
	new->upper = 0;
	new->d = 0;
	for (int i = 0; i < 26; ++i)
	{
		new->chars_up[i] = 0;
		new->chars_lo[i] = 0;
	}
	for (int i = 0; i < 10; ++i)
	{
		new->digits[i] = 0;
		new->other[i] = '\n';
	}

	// traverse through the char class and update values accordingly
	int i = 0;
	while(*pat != ']'){
		if(pat[1] == '-'){
			if((pat[0] == 'a') && (pat[2] == 'z'))		// a-z
				new->lower = 1;
			else if((pat[0] == 'A') && (pat[2] == 'Z'))		// A-Z
				new->upper = 1;
			else if((pat[0] == '0') && (pat[2] == '9'))		// 0-9
				new->d = 1;
			pat = pat + 3;
		}
		else{
			if(*pat >= 'a' && (*pat <= 'z'))	// add individual lowercase letters
				new->chars_lo[*pat - 'a'] = 1;
			if(*pat >= 'A' && (*pat <= 'Z'))	// add individual uppercase letters
				new->chars_up[*pat - 'A'] = 1;
			if(*pat >= '0' && (*pat <= '9'))	// add individual digits
				new->digits[*pat - '0'] = 1;
			else
				new->other[i++] = *pat;		// add any other chars
			pat++;
		}
	}
	*end = pat - temp;	// return the length of char class/ index where it ends
	return new;
}

// match the created char class against text
int match_class_char(char* text, char_class* class){
	if(*text >= 'a' && (*text <= 'z')){
		if(class->lower)
			return 1;
		if(class->chars_lo[*text - 'a'])
			return 1;
	}
	if(*text >= 'A' && (*text <= 'Z')){
		if(class->upper)
			return 1;
		if(class->chars_up[*text - 'A'])
			return 1;
	}
	if(*text >= '0' && (*text <= '9')){
		if(class->d)
			return 1;
		if(class->digits[*text - '0'])
			return 1;
	}
	else{
		for (int i = 0; i < 10; ++i)
		{
			if(class->other[i] == *text)
				return 1;
		}
	}
	return 0;
}

// performs matching of char class in given text
int match_class(char* pat, char* text, int* i){
	int end = 0;
	char_class* class = create_class(pat, &end);
	char ch = *(pat + end + 1);
	if((ch == '*') && (*(pat + end + 2) == '?')){	// non-greedy matching
	    do {    
	        if (!match_class_char(text, class) || (*(pat + end + 3) == '\0')){
	        	free(class);
	            return match_here(pat+end+3,text,i);
	        }
	    } while (*text != '\0' && (match_class_char(text++, class)) && (++(*i)));
	    free(class);
	    return 0;		
	}
	else if(ch == '*'){		// 0 or more occurrences of chars in char class
		char *t;
		for (t = text; *t != '\0' && (match_class_char(t, class)); t++, (*i)++)
			;
		do {
			int temp = *i;	
			if (match_here(pat+end+2, t, i)){
				free(class);
				return 1;
			}
			*i = temp;
		} while ((t-- > text) && (*i)--);
		free(class);
		return 0;
	}
	else if(ch == '+'){	 // 1 or more occurrences of chars in char class
		if(*(pat+end+2) == '?'){	
		    while (*text != '\0' && (match_class_char(text++, class)) && (++(*i))) {    
				int temp = *i;
				// terminate matching once non-matching pair of chars found and resume matching from next char of pattern and text
		        if (!(match_class_char(text, class)) || match_here(pat+end+3,text,i) || (*(pat+end+3) == '\0')){
		        	if(!(match_class_char(text, class))){
		        		(*i) = temp;
		        	}
		        	free(class);
		            return match_here(pat+end+3,text,i);
		        }
		        (*i) = temp;
		        
		    }
		    free(class);
		    return 0;
		}
	    while (*text != '\0' && (match_class_char(text++, class)))
	    {   
	    	(++(*i));
			int temp = *i;
	        if (!(match_class_char(text, class)) || ((*(pat+end+2) != '\0') ? match_here(pat+end+2,text,i) : 0)){
	        	if(match_class_char(text, class))
	        		(*i) = temp;
	        	free(class);
	            return match_here(pat+end+2,text,i);
	        }
	        (*i) = temp;        
	    } 
	    free(class);
	    return 0;
	}
	else if(ch == '?'){	 	// 0 or 1 occurrences of chars in char class
	    if (*text != '\0' && (match_class_char(text, class)))
	    {   
	    	(++(*i)); 
	    	text++;   
	    } 
	    free(class);
		return match_here(pat+end+2, text, i);
	}
	else{		// match single occurrence of char in char class
		if(match_class_char(text, class)){
			(*i)++;
			free(class);
			return match_here(pat + end + 1, text + 1, i);
		} 
		free(class);
		return 0;
	}
}

int main(){
	char text[4001];
	scanf("%[^\n]%*c", text); 
	int m;
	scanf("%d\n",&m);
	for (int i = 0; i < m; ++i)
	{
		char pat[1001];
		scanf("%[^\n]%*c", pat);
		match(pat, text); 
	}
}



