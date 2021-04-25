/* pack2dx.c: Create .1 and .2DX files from .BME and .WAV files.
 *
 * NOTE: _THIS DOES NOT WORK YET_. Fixing this shit up is left as an exercise
 * for the reader. Both the .1 and .2DX stuff generates borked output atm.
 *
 * Have fun. */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Error handling
 */

/** Print an error message and exit with a generic error code */
void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

/*
 * Arrays
 */

struct array {
	int step;
	int alloced;
	int nelts;
	void *elts;
};

/** Initialise array structure */
void array_init(struct array *self, int step)
{
	self->step = step;
	self->alloced = 4;
	self->nelts = 0;
	self->elts = malloc(step * 4);
}

/** Allocate count new entries in the array, returning a pointer to the first
 * new entry. Invalidates self->elts. */
void *array_append(struct array *self, int count)
{
	if (self->nelts + count > self->alloced) {
		while (self->nelts + count > self->alloced) self->alloced <<= 1;
		self->elts = realloc(self->elts, self->alloced * self->step);
	}

	memset(self->elts + (self->step * self->nelts), 0, count * self->step);

	self->nelts += count;
	return self->elts + (self->step * (self->nelts - count));
}

/** Ensure that the array has at least count elements, filling any newly
 * introduced entries with *value. Returns a valid value for self->elts */
void *array_fill(struct array *self, int count, const void *value)
{
	int resized = 0;
	int i;

	if (count <= self->nelts) return self->elts;

	/* Allocate extra memory if necessary */
	while (self->alloced < count) {
		self->alloced <<= 1;
		resized = 1;
	}

	if (resized) {
		self->elts = realloc(self->elts, self->alloced * self->step);
	}

	/* Fill the empty space */
	for (i = self->nelts ; i < count ; i++) {
		memcpy(self->elts + (i * self->step), value, self->step);
	}

	self->nelts = count;
	return self->elts;
}

/** Dispose of the array */
void array_fini(struct array *self)
{
	free(self->elts);
}

/*
 * Hash Tables (string -> int)
 */

struct hash_node
{
	struct hash_node *next;
	int val;
	char key[1];
};

struct hash_table {
	struct hash_node *buckets[256];
};

/** 8-bit hash function for arbitrary C strings. Adapted from Bob Jenkins'
 * hash function. */
unsigned char hash_function(const char *key)
{
	int hash = 0;
	
	for ( ; *key != '\0' ; key++) {
		hash += *key;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return (hash ^ (hash >> 8) ^ (hash >> 16) ^ (hash >> 24));
}

/** Initialise a hash table */
void hash_init(struct hash_table *self)
{
	memset(self, 0, sizeof(struct hash_table));
}

/** Look up an integer key in the hash table. Returns -1 if not found. */
int hash_get(struct hash_table *self, const char *key)
{
	struct hash_node *node;
	unsigned char bucket;

	bucket = hash_function(key);
	for (node = self->buckets[bucket] ; node != NULL ; node = node->next) {
		if (strcmp(key, node->key) == 0) {
			/* Found it */
			return node->val;
		}
	}

	/* Not found */
	return -1;
}

/** Add an entry to the hash table. Note that this function assumes that there
 * is no entry currently in the hash table for the current key. */
void hash_set(struct hash_table *self, const char *key, int val)
{
	struct hash_node *node;
	unsigned char bucket;

	bucket = hash_function(key);

	/* Allocate new node */
	node = malloc(sizeof(struct hash_node) + strlen(key));
	node->next = self->buckets[bucket];
	node->val = val;
	strcpy(node->key, key);

	/* Prepend to bucket */
	bucket = hash_function(key);
	self->buckets[bucket] = node;
}

/** Hash table cleanup */
void hash_fini(struct hash_table *self)
{
	struct hash_node *node, *next;
	int i;

	for (i = 0 ; i < 256 ; i++) {
		node = self->buckets[i];
		next = node != NULL ? node->next : NULL;

		while (node != NULL) {
			next = node->next;
			free(node);

			node = next;
		}
	}
}

/*
 * Keysound registry and .2dx construction
 */

struct keysound {
	int id;
	char filename[1];
};

struct ks_registry {
	struct hash_table filenames;
	struct array keysounds;
};

/** Initialise the keysound registry */
void ks_init(struct ks_registry *self)
{
	hash_init(&self->filenames);
	array_init(&self->keysounds, sizeof(void *));
}

/** Register a WAV file with the keysound registry, or look up an existing
 * entry. Returns the integer identifier for the keysound, starting from 1 */
int ks_register(struct ks_registry *self, const char *filename)
{
	struct keysound **pks;
	struct keysound *ks;
	int id;

	/* See if we've already allocated a slot for this file */
	id = hash_get(&self->filenames, filename);

	if (id == -1) {
		/* Allocate id */
		id = self->keysounds.nelts + 1;
		hash_set(&self->filenames, filename, id);

		/* Create keysound record */
		ks = malloc(sizeof(struct keysound) + strlen(filename));
		ks->id = id;
		strcpy(ks->filename, filename);

		pks = array_append(&self->keysounds, 1);
		*pks = ks;
	}

	return id;
}

/** Construct an unencrypted .2DX file from the contents of the keysound
 * registry. */
void ks_create_2dx(struct ks_registry *self, const char *filename)
{
	unsigned long subheader[] = {
		0x39584432,		/* "2DX9" Little Endian */
		0x00000000,		/* Insert filesize here */
		0xFFFF3230,		/* Arbitrary constant */
		0x00080040,		/* Sometimes 0x00010040 */
		0x00000000		/* Uh, zero I guess */
	};

	unsigned long wav_length;
	struct keysound **keysounds = self->keysounds.elts;
	long nkeysounds = self->keysounds.nelts;
	long offset = 0x48 + 4 * nkeysounds;
	void *wav_data;
	FILE *wav;
	FILE *out;
	int i;

	out = fopen(filename, "wb");
	if (out == NULL) die("Could not create .2dx file %s\n", filename);

	/* Write keysound count */
	fseek(out, 0x14, SEEK_SET);
	fwrite(&nkeysounds, 4, 1, out);

	for (i = 0 ; i < nkeysounds ; i++) {
		/* Write offset to TOC */
		fseek(out, 0x48 + (i * 4), SEEK_SET);
		fwrite(&offset, 4, 1, out);

		/* Open WAV */
		wav = fopen(keysounds[i]->filename, "rb");
		if (wav == NULL) die("Could not open keysound WAV %s\n",
			keysounds[i]->filename);

		/* Determine WAV length */
		fseek(wav, 0, SEEK_END);
		wav_length = ftell(wav);
		fseek(wav, 0, SEEK_SET);

		subheader[1] = wav_length;
		wav_data = malloc(wav_length);
		fread(wav_data, 1, wav_length, wav);

		/* Output subheader and WAV data at offset */
		fseek(out, offset, SEEK_SET);
		fwrite(subheader, 1, sizeof(subheader), out);
		fwrite(wav_data, 1, wav_length, out);

		/* Advance offset */
		offset += wav_length;

		/* Cleanup */
		free(wav_data);
		fclose(wav);
	}
}

/** Clean up keysound registry */
void ks_fini(struct ks_registry *self)
{
	struct keysound **keysounds = self->keysounds.elts;
	int i;

	for (i = 0 ; i < self->keysounds.nelts ; i++) {
		free(keysounds[i]);
	}

	hash_fini(&self->filenames);
	array_fini(&self->keysounds);
}

/*
 * BMS Data Structures
 */

enum bms_ev_type { BMS_EV_AUTO, BMS_EV_NOTE, BMS_EV_BPM };

struct bms_ev_auto {
	int keysound;
};

struct bms_ev_note {
	int keysound;
	int lane;
};

struct bms_ev_bpm {
	int bpm;
};

struct bms_ev {
	enum bms_ev_type type;
	double measure;
	double sec;
	union {
		struct bms_ev_auto ev_auto;
		struct bms_ev_note ev_note;
		struct bms_ev_bpm ev_bpm;
	} ev;
};

struct bms_measure {
	double beats;
	double start_sec;
};

struct bms {
	struct hash_table wavs;
	struct hash_table bpms;
	struct array events;
	struct array measures;
	const char *filename;
	int max_measure;
	int start_bpm;
};

/*
 * BMS Parser
 */

/** Parse a measure multiplier. Damn that's some hacky syntax */
void bms_measure_mul(struct bms *self, int measure, char *str)
{
	static const struct bms_measure default_measure = { 4.0, 0 };
	struct bms_measure *measures;

	measures = array_fill(&self->measures, measure, &default_measure);
	measures[measure].beats = atof(str) * 4.0;
}

/** Parse a list of events. Yes I'm aware that this is a horrible mess,
 * although having said that I've tried to make it less so. Still, hey, it's
 * BMS. It's a mess anyway. */
void bms_block_list(struct bms *self, struct ks_registry *registry,
	int measure, int channel, char *body)
{
	static int lane_map[] = { 1, 2, 3, 4, 5, 0, 0, 6, 7 };

	char block[3] = { 0, 0, 0 }; /* Null terminate it */
	struct bms_ev *ev;
	double step;
	int nblocks;
	int nchars;
	int lane;
	int bpm;
	int id;
	int i;

	self->max_measure = 
		self->max_measure < measure ? measure : self->max_measure;

	nblocks = strlen(body) / 2;
	nchars = nblocks * 2; /* Ignore any odd dangling crap */
	step = 1.0 / nblocks;

	for (i = 0 ; i < nchars ; i += 2) { 
		block[0] = body[i];
		block[1] = body[i] + 1;

		/* Skip zeros */
		if (block[0] == '0' && block[1] == '0') continue;

		switch (channel) {
		case 1: 
			/* Auto sound */
			id = hash_get(&self->wavs, block);
			if (id == -1) continue; 

			ev = array_append(&self->events, 1);
			ev->measure = measure + (i * step);
			ev->type = BMS_EV_AUTO;
			ev->ev.ev_auto.keysound = id;

			break;

		case 3:
			/* Hex BPM */
			bpm = 0;
			sscanf(block, "%x", &bpm); /* Parse hex BPM into bpm */
			if (bpm == 0) continue;

			ev = array_append(&self->events, 1);
			ev->measure = measure + (i * step);
			ev->type = BMS_EV_BPM;
			ev->ev.ev_bpm.bpm = bpm;

			break;

		case 9:
			/* Referenced BPM */
			bpm = hash_get(&self->bpms, block);
			if (bpm == -1) continue;

			ev = array_append(&self->events, 1);
			ev->measure = measure + (i * step);
			ev->type = BMS_EV_BPM;
			ev->ev.ev_bpm.bpm = bpm;

			break;

		case 11: case 12: case 13: case 14: case 15: case 16: case 18: case 19:
			/* P1 note */
			lane = lane_map[channel - 11];
			id = hash_get(&self->wavs, block);
			if (id == -1) id = 0;

			ev = array_append(&self->events, 1);
			ev->measure = measure + (i * step);
			ev->type = BMS_EV_NOTE;
			ev->ev.ev_note.keysound = id;
			ev->ev.ev_note.lane = lane;

			break;

		case 21: case 22: case 23: case 24: case 25: case 26: case 28: case 29:
			/* P2 note */
			lane = lane_map[channel - 21];
			id = hash_get(&self->wavs, block);
			if (id == -1) i = 0;

			ev = array_append(&self->events, 1);
			ev->measure = measure + (i * step);
			ev->type = BMS_EV_NOTE;
			ev->ev.ev_note.keysound = id;
			ev->ev.ev_note.lane = lane;

			break;
		}
	}
}

/** Parse a #WAV declaration */
void bms_wav_def(struct bms *self, struct ks_registry *registry, char *line)
{
	char *filename;
	char *tag;
	int id;

	/* Split into tag and filename strings */
	tag = line + 4;
	filename = line + 7;
	tag[2] = '\0';

	/* Look up the keysound ID for this filename in the global keysound
     * registry. Then associate that ID with the WAV tag */
	id = ks_register(registry, filename);
	hash_set(&self->wavs, tag, id);
}

/** Parse the starting BPM declaration */
void bms_start_bpm(struct bms *self, char *line)
{
	double bpm;

	sscanf(line, "#BPM %lf", &bpm);
	self->start_bpm = (int) bpm; /* BPM must be an integer */
}

/** Parse a #BPMxx BPM tag declaration */
void bms_bpm_def(struct bms *self, char *line)
{
	char *str;
	char *tag;
	int bpm;

	tag = line + 5;
	str = line + 8;
	tag[2] = '\0';

	/* BPM must be an integer */
	bpm = (int) atof(str);
	hash_set(&self->bpms, tag, bpm);
}

void bms_parse_line(struct bms *self, struct ks_registry *registry, char *line)
{
	int measure;
	int channel;
	char c;

	if (sscanf(line, "#%3d%2d%c", &measure, &channel, &c) == 3 && c == ':') {
		/* Probably a block list, unless... */
		if (channel == 2) {
			/* Measure multiplier is a special case */
			bms_measure_mul(self, measure, line + 7);
		} else {
			bms_block_list(self, registry, measure, channel, line + 7);
		}

	} else if (strncmp(line, "#WAV", 4) == 0) {
		/* Wav def */
		bms_wav_def(self, registry, line);

	} else if (strncmp(line, "#BPM ", 5) == 0) {
		/* Starting BPM: note the space */
		bms_start_bpm(self, line);
	
	} else if (strncmp(line, "#BPM", 4) == 0) {
		/* BPM def */
		bms_bpm_def(self, line);

	} else {
		/* Something we don't care about */
	}
}

/** Translate measure positions to absolute time */
void bms_measure_to_sec(struct bms *self)
{
	struct bms_measure *measures = self->measures.elts;
	struct bms_ev *events = self->events.elts;
	double ms_beat = 0.0;	/* Measure started at this beat */
	double ss_beat = 0.0;	/* BPM segment started at this beat */
	double ss_sec = 0.0;	/* BPM segment started at this sec */
	double sr_beat;		/* Segment relative beat (beats since start of seg) */
	double beat;
	double spb = 60.0 / self->start_bpm; /* Seconds per beat */
	int measure = 0;
	int i;

	for (i = 0 ; i < self->events.nelts ; i++) {
		while (events[i].measure > measure) {
			ms_beat += measures[measure++].beats;
			measures[measure].start_sec = ss_sec + (ms_beat - ss_beat) * spb;
		}

		beat = ms_beat +
			(events[i].measure - measure) / measures[measure].beats;
		sr_beat = beat - ss_beat;
		events[i].sec = ss_sec + sr_beat * spb;

		if (events[i].type == BMS_EV_BPM) {
			ss_beat = beat;
			ss_sec = events[i].sec;
			spb = 60.0 / events[i].ev.ev_bpm.bpm;
		}
	}

	for ( ; measure < self->measures.nelts ; measure++) {
		ms_beat += measures[measure++].beats;
		measures[measure].start_sec = ss_sec + (ms_beat - ss_beat) * spb;
	}
}

/** Main BMS parsing function. Keysounds are registered in a common registry */
struct bms *bms_read(const char *filename, struct ks_registry *registry)
{
	static const struct bms_measure default_measure = { 4.0, 0 };
	struct bms *self;
	char buf[16384];
	FILE *f;
	int i;

	f = fopen(filename, "r");
	if (f == NULL) return NULL;

	self = calloc(sizeof(struct bms), 1);
	self->filename = filename;

	hash_init(&self->wavs);
	hash_init(&self->bpms);
	array_init(&self->events, sizeof(struct bms_ev));
	array_init(&self->measures, sizeof(struct bms_measure));

	while (!feof(f)) {
		/* Read a line and cut off any trailing CR/LF's */
		fgets(buf, sizeof(buf), f);
		if (buf[0] != '#') continue; /* Skip non-directive lines */

		for (i = 0 ; buf[i] != '\0' ; i++) {
			if (buf[i] == '\r' || buf[i] == '\n') buf[i] = '\0';
		}

		/* Process the line */
		bms_parse_line(self, registry, buf);
	}

	if (self->start_bpm == 0) die("%s: No starting BPM given\n", filename);

	/* Pad out measure list and perform measure -> sec conversion */
	array_fill(&self->measures, self->max_measure + 2, &default_measure);
	bms_measure_to_sec(self);

	/* Clean up temporary hash tables */
	hash_fini(&self->wavs);
	hash_fini(&self->bpms);

	return self;
}

void bms_destroy(struct bms *self)
{
	array_fini(&self->events);
	array_fini(&self->measures);
	free(self);
}

/*
 * .1 Data Structures
 */

#define SYSTEM_FPS 59.94		/* NTSC refresh rate */
#define BINDING_LEAD 15			/* KS binding lead time in FRAMES */
#define CHART_FRAME(sec) ((sec) * SYSTEM_FPS)

enum chart_id {
	CHART_SP_HYPER, CHART_SP_NORMAL, CHART_SP_ANOTHER, CHART_SP_BEGINNER,
	CHART_SP_EXTRA1, CHART_SP_EXTRA2,

	CHART_DP_HYPER, CHART_DP_NORMAL, CHART_DP_ANOTHER, CHART_DP_BEGINNER,
	CHART_DP_EXTRA1, CHART_DP_EXTRA2, CHART_MAX
};

const char *bms_filenames[] = {
	"sp_hyper.bme", "sp_normal.bme", "sp_another.bme", "sp_beginner.bme",
	"sp_extra1.bme", "sp_extra2.bme",

	"dp_hyper.bme", "dp_normal.bme", "dp_another.bme", "dp_beginner.bme",
	"dp_extra1.bme", "dp_extra2.bme"
};

enum one_ev_type {
	/* .1 event types in order of sorting tiebreak priority */
	ONE_EV_NOTECOUNT,
	ONE_EV_BPM,
	ONE_EV_UNKNOWN,
	ONE_EV_TIMING,
	ONE_EV_BAR_LINE,
	ONE_EV_AUTO,
	ONE_EV_BIND_KS_1P,
	ONE_EV_BIND_KS_2P,
	ONE_EV_NOTE_1P,
	ONE_EV_NOTE_2P,
	ONE_EV_END_CHART
};

unsigned char one_ev_codes[] = {
	/* .1 event codes as written out to the file */
	0x10,		/* ONE_EV_NOTECOUNT */
	0x04,		/* ONE_EV_BPM */
	0x05,		/* ONE_EV_UNKNOWN */
	0x08,		/* ONE_EV_TIMING */
	0x0C,		/* ONE_EV_BAR_LINE */
	0x07,		/* ONE_EV_AUTO */
	0x02,		/* ONE_EV_BIND_KS_1P */
	0x03,		/* ONE_EV_BIND_KS_2P */
	0x00,		/* ONE_EV_BIND_NOTE_1P */
	0x01,		/* ONE_EV_BIND_NOTE_2P */
	0x06		/* ONE_EV_END_CHART */
};

struct one_ev {
	unsigned int frame;
	unsigned char type;
	unsigned char selector;
	unsigned short value;
};

struct one_chart {
	struct array events;
	int keys[16];
	int is_2p;
};

struct one_ev common_prolog[] = {
	/* Something or other ... */
	{ 0, ONE_EV_UNKNOWN, 0x04, 0x0004 },

	/* Timing windows (I took 0500's entries as a default, they're all the
	 * same anyway ... well, with the exception of GAMBOL and others...) */
	{ 0, ONE_EV_TIMING, 0x00, 0x00F0 },
	{ 0, ONE_EV_TIMING, 0x01, 0x00FA },
	{ 0, ONE_EV_TIMING, 0x02, 0x00FF },
	{ 0, ONE_EV_TIMING, 0x03, 0x0003 },
	{ 0, ONE_EV_TIMING, 0x04, 0x0008 },
	{ 0, ONE_EV_TIMING, 0x05, 0x0012 }
};

struct one_ev common_epilog[] = {
	{ 0x7FFFFFFF, 0x00, 0x00, 0x00 }
};

/*
 * .1 Chart Generation
 */

/** Emit note count, initial BPM, an unknown required event code, and the
 * default IIDX timing windows */ 
void chart_emit_prolog(struct one_chart *self, struct bms *bms)
{
	struct one_ev *one_ev;
	struct bms_ev *bms_events = bms->events.elts;
	int notecount_p1 = 0;
	int notecount_p2 = 0;
	int i;

	/* Count the notes for P1 and P2 */
	for (i = 0 ; i < bms->events.nelts ; i++) {
		if (bms_events[i].type != BMS_EV_NOTE) continue;

		if (bms_events[i].ev.ev_note.lane < 8) {
			notecount_p1++;
		} else {
			notecount_p2++;
		}
	}

	/* Emit notecount */
	one_ev = array_append(&self->events, 1);
	one_ev->type = ONE_EV_NOTECOUNT;
	one_ev->value = notecount_p1;

	if (notecount_p2 != 0) {
		self->is_2p = 1;

		one_ev = array_append(&self->events, 1);
		one_ev->type = ONE_EV_NOTECOUNT;
		one_ev->selector = 1;
		one_ev->value = notecount_p2;
	}

	/* Emit initial BPM */
	one_ev = array_append(&self->events, 1);
	one_ev->type = ONE_EV_BPM;
	one_ev->value = bms->start_bpm;

	if (notecount_p2 != 0) {
		one_ev = array_append(&self->events, 1);
		one_ev->type = ONE_EV_BPM;
		one_ev->selector = 1;
		one_ev->value = bms->start_bpm;
	}

	/* Emit rest of prolog */
	one_ev = array_append(&self->events, 7);
	memcpy(one_ev, common_prolog, sizeof(struct one_ev) * 7);
}

/** Emit other stuff that doesn't fit anywhere else (autoplay sounds and BPM
 * changes). */
void chart_emit_misc(struct one_chart *self, struct bms *bms)
{
	struct bms_ev *bms_events = bms->events.elts;
	struct one_ev *one_ev;
	int i;

	for (i = 0 ; i < bms->events.nelts; i++) {
		switch (bms_events[i].type) {
		case BMS_EV_AUTO:
			one_ev = array_append(&self->events, 1);
			one_ev->type = ONE_EV_AUTO;
			one_ev->frame = CHART_FRAME(bms_events[i].sec);
			one_ev->value = bms_events[i].ev.ev_auto.keysound;

			break;

		case BMS_EV_BPM:
			one_ev = array_append(&self->events, 1);
			one_ev->type = ONE_EV_BPM;
			one_ev->frame = CHART_FRAME(bms_events[i].sec);
			one_ev->value = bms_events[i].ev.ev_bpm.bpm;

			if (self->is_2p) {
				one_ev = array_append(&self->events, 1);
				one_ev->type = ONE_EV_BPM;
				one_ev->frame = CHART_FRAME(bms_events[i].sec);
				one_ev->value = bms_events[i].ev.ev_bpm.bpm;
				one_ev->selector = 1;
			}

			break;

		default:
			break;
		}
	}
}

/** Emit notes and keysound rebind events from BMS (lane,keysound,time)
 * tuples */
void chart_emit_notes(struct one_chart *self, struct bms *bms)
{
	struct bms_ev *bms_events = bms->events.elts;
	struct bms_ev_note *bms_note;
	struct one_ev *one_ev;
	int frame;
	int lane;
	int i;

	for (i = 0 ; i < bms->events.nelts ; i++) {
		if (bms_events[i].type != BMS_EV_NOTE) continue;
		bms_note = &bms_events[i].ev.ev_note;

		/* Get some note properties */
		frame = CHART_FRAME(bms_events[i].sec) - BINDING_LEAD;
		frame = frame < 0 ? 0 : frame;
		lane = bms_note->lane;

		/* Check if we need to bind a different keysound */
		if (bms_note->keysound != 0 && bms_note->keysound != self->keys[lane]){
			/* Change keysound binding on this lane just before the note
			 * comes up. */

			one_ev = array_append(&self->events, 1);
			one_ev->frame = frame;
			one_ev->value = bms_note->keysound + 1;

			if (lane < 8) {
				one_ev->type = ONE_EV_BIND_KS_1P;
				one_ev->selector = lane;
			} else {
				one_ev->type = ONE_EV_BIND_KS_2P;
				one_ev->selector = lane - 8;
			}

			self->keys[lane] = lane;
		}

		/* Emit a note event in any case */
		one_ev = array_append(&self->events, 1);
		one_ev->frame = frame;

		if (lane < 8) {
			one_ev->type = ONE_EV_NOTE_1P;
			one_ev->selector = lane;
		} else {
			one_ev->type = ONE_EV_NOTE_2P;
			one_ev->selector = lane - 8;
		}
	}
}

/** What it says on the tin. */
void chart_emit_bar_lines(struct one_chart *self, struct bms *bms)
{
	struct bms_measure *measures = bms->measures.elts;
	struct one_ev *one_ev;
	int i;

	/* Final measure signifies end-of-chart so we don't draw a bar line
	 * for it. */
	for (i = 0 ; i < bms->measures.nelts - 1 ; i++) {
		one_ev = array_append(&self->events, 1);
		one_ev->frame = CHART_FRAME(measures[i].start_sec);
		one_ev->type = ONE_EV_BAR_LINE;

		if (self->is_2p) {
			one_ev = array_append(&self->events, 1);
			one_ev->frame = CHART_FRAME(measures[i].start_sec);
			one_ev->type = ONE_EV_BAR_LINE;
			one_ev->selector = 1;
		}
	}
}

/** Emit an end event and end marker */
void chart_emit_epilog(struct one_chart *self, struct bms *bms)
{
	struct bms_measure *last_measure; /* HURR */
	struct one_ev *one_ev;

	/* Locate last measure; this isn't actually a measure, instead it indicates
     * the end of the chart. */
	last_measure = bms->measures.elts;
	last_measure += bms->measures.nelts - 1;

	/* Emit first end marker */
	one_ev = array_append(&self->events, 1);
	one_ev->frame = CHART_FRAME(last_measure->start_sec);
	one_ev->type = ONE_EV_END_CHART;

	if (self->is_2p) {
		one_ev = array_append(&self->events, 1);
		one_ev->frame = CHART_FRAME(last_measure->start_sec);
		one_ev->type = ONE_EV_END_CHART;
		one_ev->selector = 1;
	}

	/* Emit second end marker */
	one_ev = array_append(&self->events, 1);
	memcpy(one_ev, common_epilog, sizeof(struct one_ev));
}

/** qsort() collation function */
int chart_ev_cmp(const void *p1, const void *p2)
{
	const struct one_ev *lhs = p1;
	const struct one_ev *rhs = p2;

	return lhs->frame != rhs->frame 
		? lhs->frame - rhs->frame : lhs->type - rhs->type;
}

/** Top-level BMS to .1 data structure conversion, this is more or less the
 * "core" of the application (alongside the .2DX packer) */
struct one_chart *chart_convert(struct bms *bms)
{
	struct one_chart *self;

	self = calloc(sizeof(struct one_chart), 1);
	array_init(&self->events, sizeof(struct one_ev));

	chart_emit_prolog(self, bms);
	chart_emit_notes(self, bms);
	chart_emit_bar_lines(self, bms);
	chart_emit_misc(self, bms);
	chart_emit_epilog(self, bms);

	/* Arrange events in the proper order */
	qsort(self->events.elts, self->events.nelts, sizeof(struct one_ev),
		chart_ev_cmp);

	return self;
}

void chart_destroy(struct one_chart *self)
{
	array_fini(&self->events);
	free(self);
}

/*
 * Driver
 */

/** Read all of the BMS files that are present, convert them into .1 event
 * lists, then write out the event lists and a TOC to a .1 file. */
void chart_create_onefile(const char *outfile, struct ks_registry *registry)
{
	struct one_ev *one_events;
	struct one_chart *one;
	struct bms *bms;
	long offset = 96;
	long length = 0;
	long zero = 0;
	FILE *f;
	int i;
	int j;

	f = fopen("output.1", "wb");
	if (f == NULL) die("Could not create output.1\n");

	for (i = 0 ; i < CHART_MAX ; i++) {
		/* Read a BMS */
		bms = bms_read(bms_filenames[i], registry);

		if (bms) {
			/* Convert the BMS to a one-chart and free the BMS */
			one = chart_convert(bms);
			one_events = one->events.elts;
			bms_destroy(bms);

			/* Compute offset and emit offset/length */
			length = one->events.nelts * 8;
			fwrite(&offset, 4, 1, f);
			fwrite(&length, 4, 1, f);

			/* Seek to chart offset and write out chart */
			fseek(f, offset, SEEK_SET);

			for (j = 0 ; j < one->events.nelts ; j++) {
				fwrite(&one_events[j].frame, 4, 1, f);
				fwrite(&one_ev_codes[one_events[j].type], 1, 1, f);
				fwrite(&one_events[j].selector, 1, 1, f);
				fwrite(&one_events[j].value, 2, 1, f);
			}

			/* Return to next header entry */
			fseek(f, (i + 1) * 8, SEEK_SET);

			/* Finalise and advance position counter */
			offset += length;
			chart_destroy(one);

		} else {
			/* No corresponding BMS, omit this entry */
			fwrite(&zero, 4, 1, f);
			fwrite(&zero, 4, 1, f);
		}
	}
}

/** Entry point. This application reads several BMS files having the fixed
 * names given in bms_filenames[], when present. The notecharts expressed in
 * these BMS files are converted into an equivalent representation in IIDX AC's
 * chart format, then combined into a IIDX .1 file. Keysound WAV files
 * referenced in these BMS files are then packed into an unencrypted .2DX */
int main(int argc, char **argv)
{
	struct ks_registry registry;

	ks_init(&registry);
	chart_create_onefile("output.1", &registry);
	ks_create_2dx(&registry, "output.2dx");

	return EXIT_SUCCESS;
}

