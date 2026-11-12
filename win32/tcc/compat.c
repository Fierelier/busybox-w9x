#include <stdio.h>
#include <string.h>
#include <getopt.h>

char *optarg;
int optind = 1, opterr = 1, optopt;
static int optpos;

static void opt_err(const char *fmt, const char *prog, const char *s, int c)
{
	if (s)
		fprintf(stderr, fmt, prog, s);
	else
		fprintf(stderr, fmt, prog, c);
	fflush(stderr);
}

static void permute(char *const *argv, int dest, int src)
{
	char **av = (char **)argv;
	char *tmp = av[src];
	int i;
	for (i = src; i > dest; i--)
		av[i] = av[i - 1];
	av[dest] = tmp;
}

static int getopt_short(int argc, char *const argv[], const char *optstring)
{
	int i;
	char c, d;
	const char *os = optstring;

	if (optind >= argc || !argv[optind])
		return -1;
	if (argv[optind][0] != '-') {
		if (optstring[0] == '-') {
			optarg = argv[optind++];
			return 1;
		}
		return -1;
	}
	if (!argv[optind][1])
		return -1;
	if (argv[optind][1] == '-' && !argv[optind][2])
		return optind++, -1;

	if (!optpos)
		optpos++;
	c = argv[optind][optpos++];
	if (!argv[optind][optpos]) {
		optind++;
		optpos = 0;
	}

	if (os[0] == '-' || os[0] == '+')
		os++;
	i = 0;
	do {
		d = os[i++];
	} while (d && d != c);

	if (d != c || c == ':') {
		optopt = c;
		if (os[0] != ':' && opterr)
			opt_err("%s: invalid option -- '%c'\n", argv[0], NULL, c);
		return '?';
	}
	if (os[i] == ':') {
		optarg = NULL;
		if (os[i + 1] != ':' || optpos) {
			optarg = argv[optind++] + optpos;
			optpos = 0;
		}
		if (optind > argc) {
			optopt = c;
			if (os[0] == ':')
				return ':';
			if (opterr)
				opt_err("%s: option requires an argument -- '%c'\n", argv[0], NULL, c);
			return '?';
		}
	}
	return c;
}

static int getopt_long_core(int argc, char *const *argv, const char *optstring,
		const struct option *longopts, int *idx, int longonly)
{
	optarg = NULL;
	if (longopts && argv[optind][0] == '-'
	 && ((longonly && argv[optind][1] && argv[optind][1] != '-')
	  || (argv[optind][1] == '-' && argv[optind][2]))
	) {
		int colon = optstring[optstring[0] == '+' || optstring[0] == '-'] == ':';
		int i, cnt, match = 0;
		char *arg = NULL, *opt, *start = argv[optind] + 1;

		for (cnt = i = 0; longopts[i].name; i++) {
			const char *name = longopts[i].name;
			opt = start;
			if (*opt == '-')
				opt++;
			while (*opt && *opt != '=' && *opt == *name)
				name++, opt++;
			if (*opt && *opt != '=')
				continue;
			arg = opt;
			match = i;
			if (!*name) {
				cnt = 1;
				break;
			}
			cnt++;
		}
		if (cnt == 1 && longonly && arg - start == 1) {
			const char *os = optstring;
			int l = arg - start;
			for (i = 0; os[i]; i++) {
				int j;
				for (j = 0; j < l && start[j] == os[i + j]; j++)
					continue;
				if (j == l) {
					cnt++;
					break;
				}
			}
		}
		if (cnt == 1) {
			i = match;
			opt = arg;
			optind++;
			if (*opt == '=') {
				if (!longopts[i].has_arg) {
					optopt = longopts[i].val;
					if (colon || !opterr)
						return '?';
					opt_err("%s: option '--%s' doesn't allow an argument\n",
						argv[0], longopts[i].name, 0);
					return '?';
				}
				optarg = opt + 1;
			} else if (longopts[i].has_arg == required_argument) {
				optarg = argv[optind];
				if (!optarg) {
					optopt = longopts[i].val;
					if (colon)
						return ':';
					if (opterr)
						opt_err("%s: option '--%s' requires an argument\n",
							argv[0], longopts[i].name, 0);
					return '?';
				}
				optind++;
			}
			if (idx)
				*idx = i;
			if (longopts[i].flag) {
				*longopts[i].flag = longopts[i].val;
				return 0;
			}
			return longopts[i].val;
		}
		if (argv[optind][1] == '-') {
			optopt = 0;
			if (!colon && opterr)
				opt_err(cnt ? "%s: option '%s' is ambiguous\n"
					: "%s: unrecognized option '%s'\n",
					argv[0], argv[optind], 0);
			optind++;
			return '?';
		}
	}
	return getopt_short(argc, argv, optstring);
}

static int getopt_long_impl(int argc, char *const *argv, const char *optstring,
		const struct option *longopts, int *idx, int longonly)
{
	int ret, skipped, resumed;

	if (!optind) {
		optpos = 0;
		optind = 1;
	}
	if (optind >= argc || !argv[optind])
		return -1;
	skipped = optind;
	if (optstring[0] != '+' && optstring[0] != '-') {
		int i;
		for (i = optind; ; i++) {
			if (i >= argc || !argv[i])
				return -1;
			if (argv[i][0] == '-' && argv[i][1])
				break;
		}
		optind = i;
	}
	resumed = optind;
	ret = getopt_long_core(argc, argv, optstring, longopts, idx, longonly);
	if (resumed > skipped) {
		int i, cnt = optind - resumed;
		for (i = 0; i < cnt; i++)
			permute(argv, skipped, optind - 1);
		optind = skipped + cnt;
	}
	return ret;
}

int getopt(int argc, char *const argv[], const char *optstring)
{
	return getopt_long_impl(argc, argv, optstring, NULL, NULL, 0);
}

int getopt_long(int argc, char *const *argv, const char *optstring,
		const struct option *longopts, int *idx)
{
	return getopt_long_impl(argc, argv, optstring, longopts, idx, 0);
}

int getopt_long_only(int argc, char *const *argv, const char *optstring,
		const struct option *longopts, int *idx)
{
	return getopt_long_impl(argc, argv, optstring, longopts, idx, 1);
}

char *strtok_r(char *s, const char *delim, char **saveptr)
{
	char *end;

	if (!s)
		s = *saveptr;
	s += strspn(s, delim);
	if (!*s) {
		*saveptr = s;
		return NULL;
	}
	end = s + strcspn(s, delim);
	if (*end)
		*end++ = '\0';
	*saveptr = end;
	return s;
}

int ffs(int i)
{
	unsigned v = i;
	int n = 1;

	if (!v)
		return 0;
	while (!(v & 1)) {
		v >>= 1;
		n++;
	}
	return n;
}

size_t strnlen(const char *s, size_t n)
{
	const char *p = memchr(s, 0, n);
	return p ? (size_t)(p - s) : n;
}
