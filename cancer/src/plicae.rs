/*
 * plicae — legit plicam integram in memoriam
 */

use std::fs;

/// Lege plicam integram in chordam. Reddit None si error.
pub fn ison_lege_plicam(via: &str) -> Option<String> {
    fs::read_to_string(via).ok()
}

#[cfg(test)]
mod probationes {
    use super::*;

    #[test]
    fn proba_plicam_non_existentem() {
        assert!(ison_lege_plicam("/non/existens").is_none());
    }
}
