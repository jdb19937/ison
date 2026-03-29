/*
 * scriptor — aedificator objectorum ISON
 *
 * Aedificat obiectum ISON incrementatim,
 * effugiens characteres speciales automatice.
 */

/// Scriptor ISON — aedificat obiectum ISON in chorda.
pub struct IsonScriptor {
    data: String,
    numerus: usize, /* paria addita */
}

/// Scribe chordam ISON cum effugio in receptaculum.
fn scribe_chordam(receptaculum: &mut String, s: &str) {
    receptaculum.push('"');
    for c in s.chars() {
        match c {
            '"' => receptaculum.push_str("\\\""),
            '\\' => receptaculum.push_str("\\\\"),
            '\n' => receptaculum.push_str("\\n"),
            '\r' => receptaculum.push_str("\\r"),
            '\t' => receptaculum.push_str("\\t"),
            c if (c as u32) < 0x20 => {
                receptaculum.push_str(&format!("\\u{:04x}", c as u32));
            }
            _ => receptaculum.push(c),
        }
    }
    receptaculum.push('"');
}

impl IsonScriptor {
    /// Crea scriptorem novum.
    pub fn crea() -> Self {
        Self {
            data: String::from("{"),
            numerus: 0,
        }
    }

    /// Adde par clavis-valor cum effugio.
    pub fn adde(&mut self, clavis: &str, valor: &str) {
        if self.numerus > 0 {
            self.data.push_str(", ");
        }
        scribe_chordam(&mut self.data, clavis);
        self.data.push_str(": ");
        scribe_chordam(&mut self.data, valor);
        self.numerus += 1;
    }

    /// Adde par clavis-valor crudum (valor non effugitur).
    pub fn adde_crudum(&mut self, clavis: &str, valor: &str) {
        if self.numerus > 0 {
            self.data.push_str(", ");
        }
        scribe_chordam(&mut self.data, clavis);
        self.data.push_str(": ");
        self.data.push_str(valor);
        self.numerus += 1;
    }

    /// Fini obiectum et redde chordam ISON.
    pub fn fini(mut self) -> String {
        self.data.push('}');
        self.data
    }
}

#[cfg(test)]
mod probationes {
    use super::*;

    #[test]
    fn proba_scriptor_simplex() {
        let mut s = IsonScriptor::crea();
        s.adde("nomen", "Marcus");
        s.adde("urbs", "Roma");
        let res = s.fini();
        assert_eq!(res, r#"{"nomen": "Marcus", "urbs": "Roma"}"#);
    }

    #[test]
    fn proba_scriptor_crudum() {
        let mut s = IsonScriptor::crea();
        s.adde("nomen", "Gaius");
        s.adde_crudum("aetas", "42");
        let res = s.fini();
        assert_eq!(res, r#"{"nomen": "Gaius", "aetas": 42}"#);
    }

    #[test]
    fn proba_effugium_in_scriptore() {
        let mut s = IsonScriptor::crea();
        s.adde("textus", "linea\naltera");
        let res = s.fini();
        assert!(res.contains("\\n"));
    }
}
