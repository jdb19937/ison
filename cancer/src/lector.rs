/*
 * lector — extrahit paria clavis-valor ex objecto ISON plano
 *
 * Legit objectum ISON primi gradus et reddit paria
 * in vectorem structurarum IsonPar.
 */

use crate::navigator::transili_valorem;

/// Par clavis-valor extractum ex ISON.
#[derive(Debug, Clone, Default)]
pub struct IsonPar {
    pub clavis: String,
    pub valor: String,
}

/// Transili spatia alba.
pub fn transili_spatia(octeti: &[u8], mut pos: usize) -> usize {
    while pos < octeti.len() && matches!(octeti[pos], b' ' | b'\t' | b'\n' | b'\r') {
        pos += 1;
    }
    pos
}

/// Lege chordam ISON ab indice (pos ad '"' initialem).
/// Reddit (chorda, index post '"' terminalem).
pub fn lege_chordam(octeti: &[u8], pos: usize) -> Option<(String, usize)> {
    if pos >= octeti.len() || octeti[pos] != b'"' {
        return None;
    }
    let mut i = pos + 1;
    let mut receptaculum = String::new();

    while i < octeti.len() && octeti[i] != b'"' {
        if octeti[i] == b'\\' {
            i += 1;
            if i >= octeti.len() {
                return None;
            }
            let c = match octeti[i] {
                b'"' => '"',
                b'\\' => '\\',
                b'/' => '/',
                b'n' => '\n',
                b'r' => '\r',
                b't' => '\t',
                b'b' => '\u{08}',
                b'f' => '\u{0c}',
                b'u' => {
                    /* transili \uXXXX, pone '?' */
                    if i + 4 < octeti.len() {
                        i += 4;
                    }
                    '?'
                }
                alia => alia as char,
            };
            receptaculum.push(c);
        } else {
            receptaculum.push(octeti[i] as char);
        }
        i += 1;
    }

    if i < octeti.len() && octeti[i] == b'"' {
        i += 1;
    }
    Some((receptaculum, i))
}

/// Lege objectum ISON planum et extrahe paria clavis-valor.
/// Reddit vectorem parium, vel None si malformatum.
pub fn ison_lege(ison: &str, max_pares: usize) -> Option<Vec<IsonPar>> {
    let octeti = ison.as_bytes();
    let mut pos = transili_spatia(octeti, 0);
    if pos >= octeti.len() || octeti[pos] != b'{' {
        return None;
    }
    pos += 1;

    let mut pares = Vec::new();

    while pares.len() < max_pares {
        pos = transili_spatia(octeti, pos);
        if pos >= octeti.len() || octeti[pos] == b'}' {
            break;
        }

        if !pares.is_empty() {
            if octeti[pos] == b',' {
                pos += 1;
            }
            pos = transili_spatia(octeti, pos);
        }

        if pos >= octeti.len() || octeti[pos] != b'"' {
            break;
        }

        /* clavis */
        let (clavis, nova_pos) = lege_chordam(octeti, pos)?;
        pos = nova_pos;

        pos = transili_spatia(octeti, pos);
        if pos >= octeti.len() || octeti[pos] != b':' {
            return None;
        }
        pos += 1;
        pos = transili_spatia(octeti, pos);

        /* valor */
        if pos >= octeti.len() {
            return None;
        }
        if octeti[pos] == b'"' {
            let (valor, nova_pos) = lege_chordam(octeti, pos)?;
            pos = nova_pos;
            pares.push(IsonPar { clavis, valor });
        } else if octeti[pos] == b'{' || octeti[pos] == b'[' {
            /* transili objecta et indices imbricata */
            pos = transili_valorem(octeti, pos);
            continue;
        } else {
            /* numeri, true, false, null */
            let initium = pos;
            while pos < octeti.len() && !matches!(octeti[pos], b',' | b'}' | b' ' | b'\t' | b'\n') {
                pos += 1;
            }
            let valor = String::from_utf8_lossy(&octeti[initium..pos]).into_owned();
            pares.push(IsonPar { clavis, valor });
        }
    }

    Some(pares)
}

#[cfg(test)]
mod probationes {
    use super::*;

    #[test]
    fn proba_lector_simplex() {
        let ison = r#"{"nomen": "Marcus", "aetas": "30"}"#;
        let pares = ison_lege(ison, 32).unwrap();
        assert_eq!(pares.len(), 2);
        assert_eq!(pares[0].clavis, "nomen");
        assert_eq!(pares[0].valor, "Marcus");
        assert_eq!(pares[1].clavis, "aetas");
        assert_eq!(pares[1].valor, "30");
    }

    #[test]
    fn proba_lector_numeri() {
        let ison = r#"{"x": 42, "y": true}"#;
        let pares = ison_lege(ison, 32).unwrap();
        assert_eq!(pares.len(), 2);
        assert_eq!(pares[0].valor, "42");
        assert_eq!(pares[1].valor, "true");
    }

    #[test]
    fn proba_lector_imbricatum() {
        let ison = r#"{"a": "1", "b": {"c": "2"}, "d": "3"}"#;
        let pares = ison_lege(ison, 32).unwrap();
        /* objectum imbricatum transilitur */
        assert_eq!(pares.len(), 2);
        assert_eq!(pares[0].clavis, "a");
        assert_eq!(pares[1].clavis, "d");
    }
}
