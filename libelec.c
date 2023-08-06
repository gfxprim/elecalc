//SPDX-License-Identifier: LGPL-2.1-or-later

/*

    Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <math.h>
#include <string.h>
#include "libelec.h"

struct elec_material elec_material[ELEC_RESISTIVITY_CNT] = {
	{"silver",          1.59e-8, 3.80e-3, 10490, "Ag"},
	{"copper",          1.68e-8, 4.04e-3,  8960, "Cu"},
	{"annealed copper", 1.72e-8, 3.93e-3,  8930, "Cu annealed"},
	{"gold",            2.44e-8, 3.40e-3, 19300, "Au"},
	{"aluminium",       2.65e-8, 3.90e-3,  2700, "Al"},
	{"brass (5% Zn)",   3.00e-8,       0,  8860, "95% Cu 5% Zn"},
	{"zinc",            5.90e-8, 3.70e-3,  7140, "Zn"},
	{"brass (30% Zn)",  5.99e-8, 1.50e-3,  8550, "70% Cu 30% Zn"},
	{"nickel",          6.99e-8, 6.00e-3,  8908, "Ni"},
	{"iron",            9.70e-8, 5.00e-3,  7874, "Fe"},
	{"platinum",        10.6e-8, 3.90e-3, 21450, "Pt"},
	{"tin",             10.9e-8, 4.50e-3,  7265, "Sn"},
	//https://ntrs.nasa.gov/api/citations/20090032058/downloads/20090032058.pdf
	{"phosphor bronze", 11.2e-8, 0.92e-3,  8860, "94.8% Cu 5% Sn 0.2% P"},
	{"carbon steel",    14.3e-8,       0,  7870, "~0.1% C ~0.45%Mn ~99%Fe"},
	{"lead",            22.0e-8, 3.90e-3, 11340, "Pb"},
	{"titanium",        42.0e-8, 3.80e-3,  4506, "Ti"},
	{"manganin",        43.0e-8, 0.002e-3, 8400, "86% Cu 12%Mn 2%Ni"},
	{"constantan",      49.0e-8, 0.008e-3, 8885, "55% Cu 45% Ni"},

	{"stainless steel 201/202",    70.0e-8, 0.94e-3, 7800, "71-68% Fe 18-17% Cr 9-7%Mn 5% Ni"},
	{"stainless steel 301/303, A1", 73.0e-8, 0.94e-3, 7900, "74-73% Fe 18% Cr 9-8% Ni"},
	{"stainless steel 304, A2",     73.0e-8, 0.94e-3, 7900, "73-70% Fe 19-18% Cr 11-9% Ni"},
	{"stainless steel 316, A4",     75.0e-8, 0.94e-3, 8000, "70-65% Fe 18-17% Cr 14-11% Ni 3-2% Mo"},

	{"mercury",           98.0e-8, 0.90e-3, 13534, "Hg"},
	{"nichrome (20% Cr)", 110e-8, 0.40e-3,  8310, "80% Ni 20% Cr"},
	// Small impurities cause high change in resistance and TCR
	{"bismuth",           129e-8,  NAN,  9780, "Bi"},
	{"manganese",         144e-8,  0.01e-3,  7210, "Mn"},
};

const struct elec_units elec_units_area[ELEC_UNIT_AREA_CNT] = {
	{"m\u00b2", 1},
	{"dm\u00b2", 0.01},
	{"cm\u00b2", 0.0001},
	{"mm\u00b2", 0.000001},
	{"AWG", NAN},
};

const struct elec_units elec_units_length[ELEC_UNIT_LENGTH_CNT] = {
	{"km", 1000},
	{"m", 1},
	{"dm", 0.1},
	{"cm", 0.01},
	{"mm", 0.001},
	{"inch", 0.0254},
	{"foot", 0.3048},
};

const struct elec_units elec_units_mass[ELEC_UNIT_MASS_CNT] = {
	{"t", 1000000},
	{"kg", 1000},
	{"g", 1},
	{"mg", 0.001},
	{"\u00b5g", 0.000001},
};

const struct elec_units elec_units_resistance[ELEC_UNIT_RESISTANCE_CNT] = {
	{"M\u03a9", 1000000},
	{"k\u03a9", 1000},
	{"\u03a9", 1},
	{"m\u03a9", 0.001}
};

const struct elec_units elec_units_voltage[ELEC_UNIT_VOLTAGE_CNT] = {
	{"kV", 1000},
	{"V", 1},
	{"mV", 0.001},
	{"\u00b5V", 0.000001}
};

const struct elec_units elec_units_current[ELEC_UNIT_CURRENT_CNT] = {
	{"kA", 1000},
	{"A", 1},
	{"mA", 0.001},
	{"\u00b5A", 0.000001},
	{"pA", 0.000000001}
};

const struct elec_units elec_units_power[ELEC_UNIT_POWER_CNT] = {
	{"MW", 1000000},
	{"kW", 1000},
	{"W", 1},
	{"mW", 0.001},
	{"\u00b5W", 0.000001},
	{"hp", 746}
};

struct elec_material *elec_material_by_name(const char *name)
{
	struct elec_material *i;

	for (i = elec_material; i->name; i++) {
		if (!strcmp(i->name, name))
			return i;
	}

	return NULL;
}

static double area_convert_to_m2(double area, size_t units_area)
{
	/* AWG */
	if (isnan(elec_units_area[units_area].mul)) {
		double d = 0.000127 * pow(92, (36-area)/39);

		return M_PI * d * d / 4;
	}

	return area * elec_units_area[units_area].mul;
}

static double elec_area_convert(double area, size_t from_units_area, size_t to_units_area)
{
	area = area_convert_to_m2(area, from_units_area);

	/* AWG */
	if (isnan(elec_units_area[to_units_area].mul)) {
		double d = sqrt(area * 4 / M_PI);

		return -39 * log(d/0.000127) / log(92) + 36;
	}

	return area / elec_units_area[to_units_area].mul;
}

static void area_convert(struct elec_val *value, elec_unit unit_to)
{
	value->val = elec_area_convert(value->val, value->unit, unit_to);
	value->unit = unit_to;
}

static void unit_convert(struct elec_val *value, const struct elec_units *units, elec_unit unit_to)
{
	double base_unit = value->val * units[value->unit].mul;

	value->val = base_unit / units[unit_to].mul;
	value->unit = unit_to;
}

void elec_unit_convert(struct elec_val *value, elec_unit unit_to)
{
	switch (value->type) {
	case ELEC_UNIT_UNDEF:
	break;
	case ELEC_UNIT_LENGTH:
		unit_convert(value, elec_units_length, unit_to);
	break;
	case ELEC_UNIT_AREA:
		area_convert(value, unit_to);
	break;
	case ELEC_UNIT_MASS:
		unit_convert(value, elec_units_mass, unit_to);
	break;
	case ELEC_UNIT_RESISTANCE:
		unit_convert(value, elec_units_resistance, unit_to);
	break;
	case ELEC_UNIT_VOLTAGE:
		unit_convert(value, elec_units_voltage, unit_to);
	break;
	case ELEC_UNIT_CURRENT:
		unit_convert(value, elec_units_current, unit_to);
	break;
	case ELEC_UNIT_POWER:
		unit_convert(value, elec_units_power, unit_to);
	break;
	}
}

static inline struct elec_val unit_convert_ret(struct elec_val val, elec_unit unit_to)
{
	elec_unit_convert(&val, unit_to);

	return val;
}

static void length_autoscale(struct elec_val *value)
{
	elec_unit_convert(value, ELEC_UNIT_M);
	elec_unit convert_to = ELEC_UNIT_M;

	if (value->val > 1000)
		convert_to = ELEC_UNIT_KM;

	if (value->val < 1)
		convert_to = ELEC_UNIT_DM;

	if (value->val < 0.1)
		convert_to = ELEC_UNIT_CM;

	if (value->val < 0.01)
		convert_to = ELEC_UNIT_MM;

	elec_unit_convert(value, convert_to);
}

static void area_autoscale(struct elec_val *value)
{
	elec_unit_convert(value, ELEC_UNIT_M2);
	elec_unit convert_to = ELEC_UNIT_M2;

	if (value->val < 1)
		convert_to = ELEC_UNIT_DM2;

	if (value->val < 0.01)
		convert_to = ELEC_UNIT_CM2;

	if (value->val < 0.0001)
		convert_to = ELEC_UNIT_MM2;

	elec_unit_convert(value, convert_to);
}

static void mass_autoscale(struct elec_val *value)
{
	elec_unit_convert(value, ELEC_UNIT_G);
	elec_unit convert_to = ELEC_UNIT_G;

	if (value->val > 1000)
		convert_to = ELEC_UNIT_kG;

	if (value->val > 1000000)
		convert_to = ELEC_UNIT_T;

	if (value->val < 1)
		convert_to = ELEC_UNIT_mG;

	if (value->val < 0.001)
		convert_to = ELEC_UNIT_uG;

	elec_unit_convert(value, convert_to);
}

static void resistance_autoscale(struct elec_val *value)
{
	elec_unit_convert(value, ELEC_UNIT_OHM);
	elec_unit convert_to = ELEC_UNIT_OHM;

	if (value->val > 1000)
		convert_to = ELEC_UNIT_kOHM;

	if (value->val > 1000000)
		convert_to = ELEC_UNIT_MOHM;

	if (value->val < 1)
		convert_to = ELEC_UNIT_mOHM;

	elec_unit_convert(value, convert_to);
}

void elec_unit_autoscale(struct elec_val *value)
{
	switch (value->type) {
	case ELEC_UNIT_UNDEF:
	break;
	case ELEC_UNIT_LENGTH:
		length_autoscale(value);
	break;
	case ELEC_UNIT_AREA:
		area_autoscale(value);
	break;
	case ELEC_UNIT_MASS:
		mass_autoscale(value);
	break;
	case ELEC_UNIT_RESISTANCE:
		resistance_autoscale(value);
	break;
	}
}

const char *elec_unit_name(const struct elec_val *value)
{
	switch (value->type) {
	case ELEC_UNIT_LENGTH:
		return elec_units_length[value->unit].name;
	case ELEC_UNIT_AREA:
		return elec_units_area[value->unit].name;
	case ELEC_UNIT_MASS:
		return elec_units_mass[value->unit].name;
	case ELEC_UNIT_RESISTANCE:
		return elec_units_resistance[value->unit].name;
	default:
		return "invalid unit";
	}
}


double elec_length_convert_to_m(double length, size_t units_length)
{
	return length * elec_units_length[units_length].mul;
}

double elec_length_convert(double length, size_t from_units_length, size_t to_units_length)
{
	length = elec_length_convert_to_m(length, from_units_length);

	return length / elec_units_length[to_units_length].mul;
}

struct elec_val elec_resistance_block(struct elec_material *material,
                                      struct elec_val length,
                                      struct elec_val cross_section)
{
	elec_unit_convert(&length, ELEC_UNIT_M);
	elec_unit_convert(&cross_section, ELEC_UNIT_M2);

	return (struct elec_val) {
		.val = material->ro * length.val / cross_section.val,
		.type = ELEC_UNIT_RESISTANCE,
		.unit = ELEC_UNIT_OHM,
	};
}

struct elec_val elec_length_block(struct elec_material *material,
                                  struct elec_val resistance,
                                  struct elec_val cross_section)
{
	elec_unit_convert(&cross_section, ELEC_UNIT_M2);
	elec_unit_convert(&resistance, ELEC_UNIT_OHM);

	return (struct elec_val) {
		.val = resistance.val * cross_section.val / material->ro,
		.unit = ELEC_UNIT_M,
		.type = ELEC_UNIT_LENGTH,
	};
}

struct elec_val elec_mass_block(struct elec_material *material,
                                struct elec_val length, struct elec_val cross_section)
{
	elec_unit_convert(&length, ELEC_UNIT_M);
	elec_unit_convert(&cross_section, ELEC_UNIT_M2);

	return (struct elec_val) {
		.val = material->density * length.val * cross_section.val,
		.unit = ELEC_UNIT_kG,
		.type = ELEC_UNIT_MASS,
	};
}

void elec_circle_diameter(struct elec_val *value, elec_unit unit_to)
{
	elec_unit_convert(value, ELEC_UNIT_M2);

	value->val = 2 * sqrt(value->val/M_PI);
	value->type = ELEC_UNIT_LENGTH;
	value->unit = ELEC_UNIT_M;

	elec_unit_convert(value, unit_to);
}

void elec_circle_area(struct elec_val *value, elec_unit unit_to)
{
	elec_unit_convert(value, ELEC_UNIT_M);

	value->val = M_PI * value->val * value->val / 4;

	value->type = ELEC_UNIT_AREA;
	value->unit = ELEC_UNIT_M2;

	elec_unit_convert(value, unit_to);
}

void elec_ohm_law(struct elec_ohm_law *ohm_law)
{
	ohm_law->r.type = ELEC_UNIT_RESISTANCE;
	ohm_law->p.type = ELEC_UNIT_POWER;
	ohm_law->i.type = ELEC_UNIT_CURRENT;
	ohm_law->u.type = ELEC_UNIT_VOLTAGE;

	if (ohm_law->r.unit == ELEC_UNIT_UNDEF && ohm_law->p.unit == ELEC_UNIT_UNDEF) {
		struct elec_val i = unit_convert_ret(ohm_law->i, ELEC_UNIT_A);
		struct elec_val u = unit_convert_ret(ohm_law->u, ELEC_UNIT_V);

		ohm_law->r.val = u.val/i.val;
		ohm_law->r.unit = ELEC_UNIT_OHM;

		ohm_law->p.val = u.val * i.val;
		ohm_law->p.unit = ELEC_UNIT_W;
	}

	if (ohm_law->r.unit == ELEC_UNIT_UNDEF && ohm_law->i.unit == ELEC_UNIT_UNDEF) {
		struct elec_val p = unit_convert_ret(ohm_law->p, ELEC_UNIT_W);
		struct elec_val u = unit_convert_ret(ohm_law->u, ELEC_UNIT_V);

		ohm_law->i.val = p.val / u.val;
		ohm_law->i.unit = ELEC_UNIT_A;

		ohm_law->r.val = u.val * u.val / p.val;
		ohm_law->r.unit = ELEC_UNIT_OHM;
	}

	if (ohm_law->r.unit == ELEC_UNIT_UNDEF && ohm_law->u.unit == ELEC_UNIT_UNDEF) {
		struct elec_val p = unit_convert_ret(ohm_law->p, ELEC_UNIT_W);
		struct elec_val i = unit_convert_ret(ohm_law->i, ELEC_UNIT_A);

		ohm_law->u.val = p.val / i.val;
		ohm_law->u.unit = ELEC_UNIT_V;

		ohm_law->r.val = p.val / (i.val * i.val);
		ohm_law->r.unit = ELEC_UNIT_OHM;
	}

	if (ohm_law->i.unit == ELEC_UNIT_UNDEF && ohm_law->u.unit == ELEC_UNIT_UNDEF) {
		struct elec_val p = unit_convert_ret(ohm_law->p, ELEC_UNIT_W);
		struct elec_val r = unit_convert_ret(ohm_law->r, ELEC_UNIT_OHM);

		ohm_law->i.val = sqrt(p.val / r.val);
		ohm_law->i.unit = ELEC_UNIT_A;

		ohm_law->u.val = sqrt(p.val * r.val);
		ohm_law->u.unit = ELEC_UNIT_V;
	}

	if (ohm_law->u.unit == ELEC_UNIT_UNDEF && ohm_law->p.unit == ELEC_UNIT_UNDEF) {
		struct elec_val i = unit_convert_ret(ohm_law->i, ELEC_UNIT_A);
		struct elec_val r = unit_convert_ret(ohm_law->r, ELEC_UNIT_OHM);

		ohm_law->u.val = i.val * r.val;
		ohm_law->u.unit = ELEC_UNIT_V;

		ohm_law->p.val = i.val * i.val * r.val;
		ohm_law->p.unit = ELEC_UNIT_W;
	}

	if (ohm_law->i.unit == ELEC_UNIT_UNDEF && ohm_law->p.unit == ELEC_UNIT_UNDEF) {
		struct elec_val u = unit_convert_ret(ohm_law->u, ELEC_UNIT_V);
		struct elec_val r = unit_convert_ret(ohm_law->r, ELEC_UNIT_OHM);

		ohm_law->i.val = u.val / r.val;
		ohm_law->i.unit = ELEC_UNIT_A;

		ohm_law->p.val = u.val * u.val / r.val;
		ohm_law->p.unit = ELEC_UNIT_W;
	}
}
