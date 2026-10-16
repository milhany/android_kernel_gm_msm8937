#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <asm/setup.h>

static char new_command_line[COMMAND_LINE_SIZE];

static int cmdline_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", new_command_line);
	return 0;
}

static int cmdline_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cmdline_proc_show, NULL);
}

static const struct file_operations cmdline_proc_fops = {
	.open		= cmdline_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_cmdline_init(void)
{
	static const char key[] = "androidboot.verifiedbootstate=";
	char *flag, *end;
	bool quoted;

	/* Keep the kernel's original command line unchanged. */
	strlcpy(new_command_line, saved_command_line,
		sizeof(new_command_line));

	/* Restore the legacy userspace-only verifiedbootstate filter. */
	flag = new_command_line;
	while (*flag) {
		while (*flag && isspace(*flag))
			flag++;
		if (!*flag)
			break;

		/* Preserve complete, possibly quoted unrelated parameters. */
		end = flag;
		quoted = false;
		while (*end && (quoted || !isspace(*end))) {
			if (*end == '"')
				quoted = !quoted;
			end++;
		}
		if (strncmp(flag, key, sizeof(key) - 1)) {
			flag = end;
			continue;
		}
		while (*end && isspace(*end))
			end++;

		/* Copy the terminator even when this is the final flag. */
		memmove(flag, end, strlen(end) + 1);
	}

	proc_create("cmdline", 0, NULL, &cmdline_proc_fops);
	return 0;
}
fs_initcall(proc_cmdline_init);
