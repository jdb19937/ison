/*
 * isonl — iterat per lineas ISON (unum objectum per lineam)
 */

use crate::lector::{ison_lege, IsonPar};

/// Iterat per lineas ISONL.
/// Pro quaque linea non vacua, legit paria et vocat functorem.
/// Reddit numerum linearum processarum.
pub fn ison_pro_quaque_linea<F>(isonl: &str, mut functor: F) -> usize
where
    F: FnMut(&[IsonPar]),
{
    let mut n = 0;
    for linea in isonl.lines() {
        let purgata = linea.trim();
        if purgata.is_empty() {
            continue;
        }
        if let Some(pares) = ison_lege(purgata, 32) {
            if !pares.is_empty() {
                functor(&pares);
            }
        }
        n += 1;
    }
    n
}

#[cfg(test)]
mod probationes {
    use super::*;

    #[test]
    fn proba_isonl() {
        let isonl = "{\"a\": \"1\"}\n{\"b\": \"2\"}\n{\"c\": \"3\"}\n";
        let mut collecta = Vec::new();
        let n = ison_pro_quaque_linea(isonl, |pares| {
            collecta.push(pares[0].clavis.clone());
        });
        assert_eq!(n, 3);
        assert_eq!(collecta, vec!["a", "b", "c"]);
    }

    #[test]
    fn proba_isonl_lineas_vacuas() {
        let isonl = "\n\n{\"x\": \"1\"}\n\n";
        let mut n_vocata = 0;
        ison_pro_quaque_linea(isonl, |_| {
            n_vocata += 1;
        });
        assert_eq!(n_vocata, 1);
    }
}
