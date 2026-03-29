/*
 * schema — lector et validator schemarum ISON
 *
 * Legit schema et validat paria clavis-valor contra definitionem.
 */

use crate::isonl::ison_pro_quaque_linea;
use crate::lector::IsonPar;
use crate::navigator::{ison_claves, ison_da_chordam, ison_da_crudum};
use crate::plicae::ison_lege_plicam;

const SCHEMA_CAMPI_MAX: usize = 32;

/// Typus campi in schemate.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Typus {
    Chorda,
    Numerus,
}

/// Unus campus schematis.
#[derive(Debug, Clone)]
pub struct SchemaCampus {
    pub nomen: String,
    pub typus: Typus,
    pub necessarium: bool,
}

/// Schema integrum.
#[derive(Debug, Clone)]
pub struct Schema {
    pub titulus: String,
    pub campi: Vec<SchemaCampus>,
}

/// Lege schema ex chorda ISON. Reddit None si error.
pub fn schema_lege(ison: &str) -> Option<Schema> {
    let titulus = ison_da_chordam(ison, "titulus").unwrap_or_default();

    let prop_crudum = ison_da_crudum(ison, "properties")?;
    let nomina = ison_claves(&prop_crudum, SCHEMA_CAMPI_MAX);

    let mut campi = Vec::new();
    for nomen in &nomina {
        if campi.len() >= SCHEMA_CAMPI_MAX {
            break;
        }
        let via_typi = format!("properties.{}.type", nomen);
        let typus = match ison_da_chordam(ison, &via_typi) {
            Some(t) if t == "integer" => Typus::Numerus,
            _ => Typus::Chorda,
        };
        campi.push(SchemaCampus {
            nomen: nomen.clone(),
            typus,
            necessarium: false,
        });
    }

    /* lege "required" indicem */
    if let Some(req_crudum) = ison_da_crudum(ison, "required") {
        let octeti = req_crudum.as_bytes();
        if !octeti.is_empty() && octeti[0] == b'[' {
            for campus in &mut campi {
                /* quaere nomen in indice */
                let quaesitum = format!("\"{}\"", campus.nomen);
                if req_crudum.contains(&quaesitum) {
                    campus.necessarium = true;
                }
            }
        }
    }

    Some(Schema { titulus, campi })
}

/// Lege schema ex plica. Reddit None si error.
pub fn schema_lege_plicam(via: &str) -> Option<Schema> {
    let ison = ison_lege_plicam(via)?;
    schema_lege(&ison)
}

/// Est valor numerus integer?
fn est_numerus(v: &str) -> bool {
    let octeti = v.as_bytes();
    if octeti.is_empty() {
        return false;
    }
    let mut i = 0;
    if octeti[i] == b'-' {
        i += 1;
    }
    if i >= octeti.len() {
        return false;
    }
    while i < octeti.len() {
        if !octeti[i].is_ascii_digit() {
            return false;
        }
        i += 1;
    }
    true
}

/// Valida paria contra schema.
/// Reddit Ok(()) si validum, Err(nuntius) si invalidum.
pub fn schema_valida(schema: &Schema, pares: &[IsonPar]) -> Result<(), String> {
    /* verifica campos necessarios */
    for campus in &schema.campi {
        if !campus.necessarium {
            continue;
        }
        let inventum = pares.iter().any(|p| p.clavis == campus.nomen);
        if !inventum {
            return Err(format!("campus necessarius deest: \"{}\"", campus.nomen));
        }
    }

    /* verifica typum cuiusque campi */
    for par in pares {
        let campus = match schema.campi.iter().find(|c| c.nomen == par.clavis) {
            Some(c) => c,
            None => {
                return Err(format!("campus ignotus: \"{}\"", par.clavis));
            }
        };
        if campus.typus == Typus::Numerus && !est_numerus(&par.valor) {
            return Err(format!(
                "campus \"{}\": expectatur numerus, datum \"{}\"",
                campus.nomen, par.valor
            ));
        }
    }

    Ok(())
}

/// Valida plicam ISONL contra schema.
/// Reddit numerum errorum.
pub fn schema_valida_isonl(schema: &Schema, isonl: &str) -> usize {
    let mut linea_num = 0usize;
    let mut errores = 0usize;

    ison_pro_quaque_linea(isonl, |pares| {
        linea_num += 1;
        if let Err(error) = schema_valida(schema, pares) {
            eprintln!("  linea {}: {}", linea_num, error);
            errores += 1;
        }
    });

    errores
}

#[cfg(test)]
mod probationes {
    use super::*;

    #[test]
    fn proba_schema_lege() {
        let ison = r#"{
            "titulus": "Persona",
            "properties": {
                "nomen": {"type": "string"},
                "aetas": {"type": "integer"}
            },
            "required": ["nomen"]
        }"#;
        let schema = schema_lege(ison).unwrap();
        assert_eq!(schema.titulus, "Persona");
        assert_eq!(schema.campi.len(), 2);
        assert_eq!(schema.campi[0].nomen, "nomen");
        assert_eq!(schema.campi[0].typus, Typus::Chorda);
        assert!(schema.campi[0].necessarium);
        assert_eq!(schema.campi[1].nomen, "aetas");
        assert_eq!(schema.campi[1].typus, Typus::Numerus);
        assert!(!schema.campi[1].necessarium);
    }

    #[test]
    fn proba_validatio_bona() {
        let ison = r#"{
            "titulus": "Res",
            "properties": {
                "nomen": {"type": "string"},
                "numerus": {"type": "integer"}
            },
            "required": ["nomen"]
        }"#;
        let schema = schema_lege(ison).unwrap();
        let pares = vec![
            IsonPar {
                clavis: "nomen".into(),
                valor: "Marcus".into(),
            },
            IsonPar {
                clavis: "numerus".into(),
                valor: "42".into(),
            },
        ];
        assert!(schema_valida(&schema, &pares).is_ok());
    }

    #[test]
    fn proba_campus_deest() {
        let ison = r#"{
            "titulus": "Res",
            "properties": {
                "nomen": {"type": "string"}
            },
            "required": ["nomen"]
        }"#;
        let schema = schema_lege(ison).unwrap();
        let pares: Vec<IsonPar> = vec![];
        let err = schema_valida(&schema, &pares).unwrap_err();
        assert!(err.contains("deest"));
    }

    #[test]
    fn proba_typus_malus() {
        let ison = r#"{
            "titulus": "Res",
            "properties": {
                "aetas": {"type": "integer"}
            },
            "required": []
        }"#;
        let schema = schema_lege(ison).unwrap();
        let pares = vec![IsonPar {
            clavis: "aetas".into(),
            valor: "abc".into(),
        }];
        let err = schema_valida(&schema, &pares).unwrap_err();
        assert!(err.contains("expectatur numerus"));
    }
}
