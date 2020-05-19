
/**************************************************
* MultiPlugin++ by Deathspike
*
* Client-side plugin project which allows any
* developer to use this framework as a basis for
* their own plugin.
**************************************************/

#include <multiplugin/mppHeader.h>

/**************************************************
* Com_ConcatArgs
* 
* Connects all the input variables inside a long
* but simple to use string. This is often used to
* catch data such as a text message.
**************************************************/

char *Cmd_ConcatArgs( int start )
{
	int			i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int			len;
	char		*arg;

	len = 0;
	c = Cmd_Argc();

	for ( i = start ; i < c ; i++ )
	{
		arg = Cmd_Argv( i );
		tlen = strlen( arg );

		if ( len + tlen >= MAX_STRING_CHARS - 1 )
		{
			break;
		}

		memcpy( line + len, arg, tlen );
		len += tlen;

		if ( i != c - 1 )
		{
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;
	return line;
}

/**************************************************
* Com_StringEscape
*
* Escapes a string from the in buffer to the out
* buffer, altering newlines to become actual
* new lines.
**************************************************/

void Com_StringEscape(char *in, char *out)
{
	char	ch, ch1 = 0;
	int		len = 0;
	int		Size = 1023;

	while (1)
	{
		ch = *in++;
		ch1 = *in;

		if (ch == '\\' && ch1 == 'n')
		{
			in++;
			*out++ = '\n';
		}
		else
		{
			*out++ = ch;
		}

		if (len > Size - 1)
		{
			break;
		}

		len++;
	}

	return;
}
