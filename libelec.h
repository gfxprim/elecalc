//SPDX-License-Identifier: LGPL-2.1-or-later

/*

    Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef LIBELEC_H
#define LIBELEC_H

struct elec_material {
	const char *name;

	/* resistance and resistance temp coef */
	double ro;
	double tc;

	/* kg/m3 */
	double density;

	/* chemical conposition */
	const char *composition;
};

#define ELEC_RESISTIVITY_CNT 26

typedef unsigned int elec_unit;

struct elec_units {
	const char *name;
	double mul;
};

enum elec_unit_length {
	ELEC_UNIT_KM = 0,
	ELEC_UNIT_M,
	ELEC_UNIT_DM,
	ELEC_UNIT_CM,
	ELEC_UNIT_MM,
	ELEC_UNIT_INCH,
	ELEC_UNIT_FOOT,
	ELEC_UNIT_LENGTH_CNT,
};

extern const struct elec_units elec_units_length[ELEC_UNIT_LENGTH_CNT];

enum elec_unit_area {
	ELEC_UNIT_M2 = 0,
	ELEC_UNIT_DM2,
	ELEC_UNIT_CM2,
	ELEC_UNIT_MM2,
	ELEC_UNIT_AWG,
	ELEC_UNIT_AREA_CNT,
};

extern const struct elec_units elec_units_area[ELEC_UNIT_AREA_CNT];

enum elec_unit_mass {
	ELEC_UNIT_T,
	ELEC_UNIT_kG,
	ELEC_UNIT_G,
	ELEC_UNIT_mG,
	ELEC_UNIT_uG,
	ELEC_UNIT_MASS_CNT,
};

extern const struct elec_units elec_units_mass[ELEC_UNIT_MASS_CNT];

enum elec_unit_resistance {
	ELEC_UNIT_MOHM,
	ELEC_UNIT_kOHM,
	ELEC_UNIT_OHM,
	ELEC_UNIT_mOHM,
	ELEC_UNIT_RESISTANCE_CNT,
};

extern const struct elec_units elec_units_resistance[ELEC_UNIT_RESISTANCE_CNT];

enum elec_unit_voltage {
	ELEC_UNIT_KV,
	ELEC_UNIT_V,
	ELEC_UNIT_MV,
	ELEC_UNIT_UV,
	ELEC_UNIT_VOLTAGE_CNT,
};

extern const struct elec_units elec_units_voltage[ELEC_UNIT_VOLTAGE_CNT];

enum elec_unit_current {
	ELEC_UNIT_kA,
	ELEC_UNIT_A,
	ELEC_UNIT_mA,
	ELEC_UNIT_uA,
	ELEC_UNIT_pA,
	ELEC_UNIT_CURRENT_CNT,
};

extern const struct elec_units elec_units_current[ELEC_UNIT_CURRENT_CNT];

enum elec_unit_power {
	ELEC_UNIT_MW,
	ELEC_UNIT_kW,
	ELEC_UNIT_W,
	ELEC_UNIT_mW,
	ELEC_UNIT_uW,
	ELEC_UNIT_ELECTRICAL_HP,
	ELEC_UNIT_POWER_CNT,
};

extern const struct elec_units elec_units_power[ELEC_UNIT_POWER_CNT];

enum elec_unit {
	ELEC_UNIT_UNDEF,
	ELEC_UNIT_LENGTH,
	ELEC_UNIT_AREA,
	ELEC_UNIT_MASS,
	ELEC_UNIT_RESISTANCE,
	ELEC_UNIT_VOLTAGE,
	ELEC_UNIT_CURRENT,
	ELEC_UNIT_POWER,
};

struct elec_val {
	enum elec_unit type;
	double val;
	elec_unit unit;
};

extern struct elec_material elec_material[];
extern size_t elec_material_cnt;

struct elec_material *elec_material_by_name(const char *name);

/**
 * Coverts value into an SI unit and scales the value to the closest commonly
 * used SI prefix.
 */
void elec_unit_autoscale(struct elec_val *value);

/**
 * Returns name of the unit including the SI prefix.
 */
const char *elec_unit_name(const struct elec_val *value);

/**
 * Converts value into a specified unit.
 */
void elec_unit_convert(struct elec_val *value, elec_unit unit_to);

/**
 * Computes circle diameter from area and converts the result to requested
 * unit.
 */
void elec_circle_diameter(struct elec_val *value, elec_unit unit_to);

/**
 * Computes circle area from diameter and converts the result to requested
 * unit.
 */
void elec_circle_area(struct elec_val *value, elec_unit unit_to);

/**
 * Calculates resistivity of a material based on length, material, and cross section.
 *
 * @material A material description.
 * @length A material lenght.
 * @cross_section A material cross section.
 *
 * @return Resistance in Ohms.
 */
struct elec_val elec_resistance_block(struct elec_material *material,
                                      struct elec_val length, struct elec_val cross_section);

/**
 * Calculates length of a material based on resistance, material and cross section.
 *
 * @material A material description.
 * @resistance A material resistance.
 * @cross_section A material cross section.
 *
 * @return A length in meters.
 */
struct elec_val elec_length_block(struct elec_material *material,
		                  struct elec_val resistance, struct elec_val cross_section);

/**
 * Calculates material mass based on lenght, material and cross section.
 *
 * @material A material description.
 * @length A material lenght.
 * @cross_section A material cross section.
 *
 * @return A mass in kilograms.
 */
struct elec_val elec_mass_block(struct elec_material *material,
                                struct elec_val length, struct elec_val cross_section);

/**
 * The ohm law is solved for the value with unit set to ELEC_UNIT_UNDEF
 */
struct elec_ohm_law {
	struct elec_val r;
	struct elec_val i;
	struct elec_val u;
	struct elec_val p;
};

void elec_ohm_law(struct elec_ohm_law *ohm_law);

/**
 * This is solved for the value with unit set to ELEC_UNIT_UNDEF
 */
struct elec_el_power {
	struct elec_val i;
	struct elec_val u;
	struct elec_val p;
	struct elec_val r;
};

void elec_el_power(struct elec_el_power *el_power);

#endif /* LIBELEC_H */
