#include "libutil/stdafx.h"

#include "libutil/config.h"
#include "libutil/eamuse.h"
#include "libutil/input.h"

#define CONFIG_TAG_LEN 8

struct config_header
{
	int nbindings;
	int nattrs;
	wchar_t eam_machine_id_path[MAX_PATH];
	wchar_t eam_card_paths[2][MAX_PATH];
};

struct config_attr
{
	char tag[CONFIG_TAG_LEN];
	int value;
};

static struct config_header hdr;
static const wchar_t config_subdir[] = L"\\DJ Hackers";
static wchar_t config_path[MAX_PATH];
static struct config_attr *attrs;

static void config_load_header(FILE *f)
{
	fread(&hdr, sizeof(hdr), 1, f);

	eam_set_card_path(0, hdr.eam_card_paths[0]);
	eam_set_card_path(1, hdr.eam_card_paths[1]);
	eam_set_machine_id_path(hdr.eam_machine_id_path);
}

static void config_load_input(FILE *f, int nbindings)
{
	struct input_binding *bindings;

	bindings = malloc(sizeof(struct input_binding) * nbindings);
	fread(bindings, sizeof(struct input_binding), nbindings, f);

	/* Pass bindings to input subsystem, which takes ownership of this array. */
	input_set_bindings(bindings, nbindings);
}

static void config_load_attrs(FILE *f, int nattrs)
{
	attrs = malloc(sizeof(struct config_attr) * nattrs);
	fread(attrs, sizeof(struct config_attr), nattrs, f);
}

static void config_save_header(FILE *f)
{
	hdr.nbindings = input_get_nbindings();

	wcsncpy(hdr.eam_card_paths[0], eam_get_card_path(0), MAX_PATH);
	wcsncpy(hdr.eam_card_paths[1], eam_get_card_path(1), MAX_PATH);
	wcsncpy(hdr.eam_machine_id_path, eam_get_machine_id_path(), MAX_PATH);

	fwrite(&hdr, sizeof(hdr), 1, f);
}

static void config_save_input(FILE *f, int nbindings)
{
	const struct input_binding *bindings;

	bindings = input_get_bindings();
	fwrite(bindings, sizeof(struct input_binding), nbindings, f);
}

static void config_save_attrs(FILE *f, int nattrs)
{
	fwrite(attrs, sizeof(struct config_attr), nattrs, f);
}

void config_init(const wchar_t *game_type)
{
	wchar_t config_dir[MAX_PATH];
	PIDLIST_ABSOLUTE pidl;
	HRESULT hr;

	/* Get Application Data directory */
	hr = SHGetFolderLocation(NULL, CSIDL_APPDATA, NULL, 0, &pidl);
	if (FAILED(hr)) {
		panic(L"Error locating Application Data directory: %#08x", hr);
	}

	/* Translate to path */
	SHGetPathFromIDList(pidl, config_dir);
	ILFree(pidl);

	/* Ensure our subdirectory exists */
	wcscat_s(config_dir, MAX_PATH, config_subdir);
	if (!CreateDirectory(config_dir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		panic(L"Failed to create Application Data subdirectory at \"%s\"\n");
	}

	/* Form configuration filename */
	swprintf_s(config_path, MAX_PATH, L"%s\\%s_v%02d.cfg", config_dir, game_type, LIBUTIL_VERSION);

	/* Initialise the configuration header in case we can't load it */
	memset(&hdr, 0, sizeof(hdr));
}

int config_get_attr(const char *tag)
{
	int i;

	for (i = 0 ; i < hdr.nattrs ; i++) {
		if (strncmp(tag, attrs[i].tag, CONFIG_TAG_LEN) == 0) {
			return attrs[i].value;
		}
	}

	return 0;
}

void config_load(void)
{
	FILE *f;

	f = _wfopen(config_path, L"rb");

	if (f == NULL) {
		return;
	}

	config_load_header(f);
	config_load_input(f, hdr.nbindings);
	config_load_attrs(f, hdr.nattrs);

	fclose(f);
}

void config_save(void)
{
	FILE *f;

	f = _wfopen(config_path, L"wb");

	if (f == NULL) {
		panic(L"Error opening configuration file at \"%s\" for writing!\n", config_path);
	}
	
	config_save_header(f);
	config_save_input(f, hdr.nbindings);
	config_save_attrs(f, hdr.nattrs);

	fclose(f);
}

void config_set_attr(const char *tag, int value)
{
	int i;

	/* Try to alter existing attr */
	for (i = 0 ; i < hdr.nattrs ; i++) {
		if (strncmp(tag, attrs[i].tag, CONFIG_TAG_LEN) == 0) {
			attrs[i].value = value;
			return;
		}
	}

	/* Failing that, add a new one */
	hdr.nattrs++;
	attrs = realloc(attrs, sizeof(struct config_attr) * hdr.nattrs);
	attrs[hdr.nattrs - 1].value = value;
	strncpy(attrs[hdr.nattrs - 1].tag, tag, CONFIG_TAG_LEN);
}

void config_fini(void)
{
	free(attrs);
	attrs = NULL;
}
