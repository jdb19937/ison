/*
 * navigator — per viam punctatam ("a.b[0].c") navigat ad valorem ISON
 *
 * Recursivo descensu chordam ISON perscrutat,
 * sine arbore intermedia aedificanda.
 */

use crate::lector::{transili_spatia, lege_chordam};

/// Transili chordam ISON (pos ad '"' initialem). Reddit indicem post '"' terminalem.
pub fn transili_chordam(octeti: &[u8], pos: usize) -> usize {
    if pos >= octeti.len() || octeti[pos] != b'"' {
        return pos;
    }
    let mut i = pos + 1;
    while i < octeti.len() {
        if octeti[i] == b'\\' {
            i += 2;
            continue;
        }
        if octeti[i] == b'"' {
            return i + 1;
        }
        i += 1;
    }
    i
}

/// Transili quemlibet valorem ISON. Reddit indicem post valorem.
pub fn transili_valorem(octeti: &[u8], pos: usize) -> usize {
    let p = transili_spatia(octeti, pos);
    if p >= octeti.len() {
        return p;
    }
    match octeti[p] {
        b'"' => transili_chordam(octeti, p),
        b'{' => {
            let mut i = transili_spatia(octeti, p + 1);
            while i < octeti.len() && octeti[i] != b'}' {
                /* clavis */
                i = transili_chordam(octeti, transili_spatia(octeti, i));
                i = transili_spatia(octeti, i);
                if i < octeti.len() && octeti[i] == b':' {
                    i += 1;
                }
                /* valor */
                i = transili_valorem(octeti, i);
                i = transili_spatia(octeti, i);
                if i < octeti.len() && octeti[i] == b',' {
                    i = transili_spatia(octeti, i + 1);
                }
            }
            if i < octeti.len() && octeti[i] == b'}' {
                i + 1
            } else {
                i
            }
        }
        b'[' => {
            let mut i = transili_spatia(octeti, p + 1);
            while i < octeti.len() && octeti[i] != b']' {
                i = transili_valorem(octeti, i);
                i = transili_spatia(octeti, i);
                if i < octeti.len() && octeti[i] == b',' {
                    i = transili_spatia(octeti, i + 1);
                }
            }
            if i < octeti.len() && octeti[i] == b']' {
                i + 1
            } else {
                i
            }
        }
        b't' => p + 4, /* true */
        b'f' => p + 5, /* false */
        b'n' => p + 4, /* null */
        _ => {
            /* numerus */
            let mut i = p;
            if i < octeti.len() && octeti[i] == b'-' {
                i += 1;
            }
            while i < octeti.len()
                && matches!(
                    octeti[i],
                    b'0'..=b'9' | b'.' | b'e' | b'E' | b'+' | b'-'
                )
            {
                i += 1;
            }
            i
        }
    }
}

/// Quaere clavem in objecto. pos ad '{'. Reddit indicem ad valorem.
fn nav_in_objecto(octeti: &[u8], pos: usize, clavis: &[u8]) -> Option<usize> {
    let p = transili_spatia(octeti, pos);
    if p >= octeti.len() || octeti[p] != b'{' {
        return None;
    }
    let mut i = transili_spatia(octeti, p + 1);

    while i < octeti.len() && octeti[i] != b'}' {
        if octeti[i] != b'"' {
            return None;
        }
        /* compara clavem */
        let k_initium = i + 1;
        let mut k_finis = k_initium;
        while k_finis < octeti.len() && octeti[k_finis] != b'"' {
            if octeti[k_finis] == b'\\' {
                k_finis += 1;
            }
            k_finis += 1;
        }
        i = k_finis + 1; /* post '"' */
        i = transili_spatia(octeti, i);
        if i < octeti.len() && octeti[i] == b':' {
            i = transili_spatia(octeti, i + 1);
        }

        if &octeti[k_initium..k_finis] == clavis {
            return Some(i); /* valor inventus */
        }

        i = transili_valorem(octeti, i);
        i = transili_spatia(octeti, i);
        if i < octeti.len() && octeti[i] == b',' {
            i = transili_spatia(octeti, i + 1);
        }
    }
    None
}

/// Quaere indicem in indice (array). pos ad '['. Reddit indicem ad valorem.
fn nav_in_indice(octeti: &[u8], pos: usize, index: usize) -> Option<usize> {
    let p = transili_spatia(octeti, pos);
    if p >= octeti.len() || octeti[p] != b'[' {
        return None;
    }
    let mut i = transili_spatia(octeti, p + 1);

    for _ in 0..index {
        if i >= octeti.len() || octeti[i] == b']' {
            return None;
        }
        i = transili_valorem(octeti, i);
        i = transili_spatia(octeti, i);
        if i < octeti.len() && octeti[i] == b',' {
            i = transili_spatia(octeti, i + 1);
        }
    }

    if i < octeti.len() && octeti[i] != b']' {
        Some(i)
    } else {
        None
    }
}

/// Naviga per viam punctatam: "a.b[0].c".
/// Reddit indicem in octetos ubi valor incipit, vel None.
fn ison_naviga(octeti: &[u8], via: &str) -> Option<usize> {
    let mut p = transili_spatia(octeti, 0);
    let via_octeti = via.as_bytes();
    let mut v = 0;

    while v < via_octeti.len() {
        if via_octeti[v] == b'.' {
            v += 1;
            continue;
        }

        if via_octeti[v] == b'[' {
            v += 1;
            let mut idx: usize = 0;
            while v < via_octeti.len() && via_octeti[v] >= b'0' && via_octeti[v] <= b'9' {
                idx = idx * 10 + (via_octeti[v] - b'0') as usize;
                v += 1;
            }
            if v < via_octeti.len() && via_octeti[v] == b']' {
                v += 1;
            }
            p = nav_in_indice(octeti, p, idx)?;
        } else {
            let v_initium = v;
            while v < via_octeti.len() && via_octeti[v] != b'.' && via_octeti[v] != b'[' {
                v += 1;
            }
            p = nav_in_objecto(octeti, p, &via_octeti[v_initium..v])?;
        }
    }

    Some(p)
}

/// Da chordam per viam punctatam. Reddit None si non inventum.
pub fn ison_da_chordam(ison: &str, via: &str) -> Option<String> {
    let octeti = ison.as_bytes();
    let p = ison_naviga(octeti, via)?;
    let p = transili_spatia(octeti, p);
    let (chorda, _) = lege_chordam(octeti, p)?;
    Some(chorda)
}

/// Da numerum per viam punctatam. Reddit None si non inventum.
pub fn ison_da_numerum(ison: &str, via: &str) -> Option<i64> {
    let octeti = ison.as_bytes();
    let p = ison_naviga(octeti, via)?;
    let p = transili_spatia(octeti, p);

    /* extrahe numerum */
    let mut finis = p;
    if finis < octeti.len() && octeti[finis] == b'-' {
        finis += 1;
    }
    while finis < octeti.len() && octeti[finis] >= b'0' && octeti[finis] <= b'9' {
        finis += 1;
    }
    let textus = std::str::from_utf8(&octeti[p..finis]).ok()?;
    textus.parse::<i64>().ok()
}

/// Da valorem crudum per viam punctatam.
pub fn ison_da_crudum(ison: &str, via: &str) -> Option<String> {
    let octeti = ison.as_bytes();
    let p = ison_naviga(octeti, via)?;
    let p = transili_spatia(octeti, p);
    let finis = transili_valorem(octeti, p);
    if finis <= p {
        return None;
    }
    Some(String::from_utf8_lossy(&octeti[p..finis]).into_owned())
}

/// Extrahe claves primi gradus objecti ISON.
pub fn ison_claves(ison: &str, max: usize) -> Vec<String> {
    let octeti = ison.as_bytes();
    let mut p = transili_spatia(octeti, 0);
    if p >= octeti.len() || octeti[p] != b'{' {
        return Vec::new();
    }
    p = transili_spatia(octeti, p + 1);

    let mut claves = Vec::new();
    while p < octeti.len() && octeti[p] != b'}' && claves.len() < max {
        if !claves.is_empty() {
            if p < octeti.len() && octeti[p] == b',' {
                p = transili_spatia(octeti, p + 1);
            }
        }
        if p >= octeti.len() || octeti[p] != b'"' {
            break;
        }
        let (clavis, nova_pos) = match lege_chordam(octeti, p) {
            Some(v) => v,
            None => break,
        };
        p = nova_pos;
        claves.push(clavis);
        p = transili_spatia(octeti, p);
        if p < octeti.len() && octeti[p] == b':' {
            p = transili_spatia(octeti, p + 1);
        }
        p = transili_valorem(octeti, p);
        p = transili_spatia(octeti, p);
    }
    claves
}

#[cfg(test)]
mod probationes {
    use super::*;

    #[test]
    fn proba_chordam_simplicem() {
        let ison = r#"{"nomen": "Marcus"}"#;
        assert_eq!(ison_da_chordam(ison, "nomen").unwrap(), "Marcus");
    }

    #[test]
    fn proba_numerum() {
        let ison = r#"{"aetas": 42}"#;
        assert_eq!(ison_da_numerum(ison, "aetas").unwrap(), 42);
    }

    #[test]
    fn proba_viam_imbricatam() {
        let ison = r#"{"a": {"b": {"c": "valor"}}}"#;
        assert_eq!(ison_da_chordam(ison, "a.b.c").unwrap(), "valor");
    }

    #[test]
    fn proba_indicem() {
        let ison = r#"{"res": [10, 20, 30]}"#;
        assert_eq!(ison_da_numerum(ison, "res[1]").unwrap(), 20);
    }

    #[test]
    fn proba_viam_compositam() {
        let ison = r#"{"electiones": [{"nuntius": {"contentum": "salve"}}]}"#;
        assert_eq!(
            ison_da_chordam(ison, "electiones[0].nuntius.contentum").unwrap(),
            "salve"
        );
    }

    #[test]
    fn proba_crudum() {
        let ison = r#"{"a": {"b": 1}}"#;
        let crudum = ison_da_crudum(ison, "a").unwrap();
        assert!(crudum.contains("\"b\""));
    }

    #[test]
    fn proba_claves() {
        let ison = r#"{"x": 1, "y": 2, "z": 3}"#;
        let claves = ison_claves(ison, 10);
        assert_eq!(claves, vec!["x", "y", "z"]);
    }

    #[test]
    fn proba_non_inventum() {
        let ison = r#"{"a": 1}"#;
        assert!(ison_da_chordam(ison, "b").is_none());
    }
}
