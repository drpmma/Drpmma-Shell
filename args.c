#include "args.h"

static bool
is_space(const char c)
{
	return c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v' || c == ' ';
}

static void
consume_whitespace(const char **input)
{
	while (is_space(**input))
	{
		(*input)++;
	}
}

char **buildargv(const char *input)
{
	char *arg;
	char *copybuf;
	int squote = 0;
	int dquote = 0;
	int bsquote = 0;
	int argc = 0;
	int maxargc = 0;
	char **argv = NULL;
	char **nargv;

	if (input != NULL)
	{
		copybuf = (char *)malloc(strlen(input) + 1);
		/* Is a do{}while to always execute the loop once.  Always return an
	 argv, even for null strings.  See NOTES above, test case below. */
		do
		{
			/* Pick off argv[argc] */
			consume_whitespace(&input);

			if ((maxargc == 0) || (argc >= (maxargc - 1)))
			{
				/* argv needs initialization, or expansion */
				if (argv == NULL)
				{
					maxargc = INITIAL_MAXARGC;
					nargv = (char **)malloc(maxargc * sizeof(char *));
				}
				else
				{
					maxargc *= 2;
					nargv = (char **)realloc(argv, maxargc * sizeof(char *));
				}
				argv = nargv;
				argv[argc] = NULL;
			}
			/* Begin scanning arg */
			arg = copybuf;
			while (*input != EOS)
			{
				if (is_space(*input) && !squote && !dquote && !bsquote)
				{
					break;
				}
				else
				{
					if (bsquote)
					{
						bsquote = 0;
						*arg++ = *input;
					}
					else if (*input == '\\')
					{
						bsquote = 1;
					}
					else if (squote)
					{
						if (*input == '\'')
						{
							squote = 0;
						}
						else
						{
							*arg++ = *input;
						}
					}
					else if (dquote)
					{
						if (*input == '"')
						{
							dquote = 0;
						}
						else
						{
							*arg++ = *input;
						}
					}
					else
					{
						if (*input == '\'')
						{
							squote = 1;
						}
						else if (*input == '"')
						{
							dquote = 1;
						}
						else
						{
							*arg++ = *input;
						}
					}
					input++;
				}
			}
			*arg = EOS;
			argv[argc] = strdup(copybuf);
			argc++;
			argv[argc] = NULL;

			consume_whitespace(&input);
		} while (*input != EOS);

		free(copybuf);
	}
	return (argv);
}

int
countargv (char * const *argv)
{
  int argc;

  if (argv == NULL)
    return 0;
  for (argc = 0; argv[argc] != NULL; argc++)
    continue;
  return argc;
}