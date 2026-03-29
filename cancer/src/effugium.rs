/*
 * effugium — effugit characteres speciales pro ISON
 */

/// Effuge chordam pro inclusione in ISON.
/// Reddit chordam novam cum characteribus specialibus effugitis.
pub fn ison_effuge(textus: &str) -> String {
    let mut res = String::with_capacity(textus.len());
    for c in textus.chars() {
        match c {
            '"'  => res.push_str("\\\""),
            '\\' => res.push_str("\\\\"),
            '\n' => res.push_str("\\n"),
            '\r' => res.push_str("\\r"),
            '\t' => res.push_str("\\t"),
            c if (c as u32) < 0x20 => {
                res.push_str(&format!("\\u{:04x}", c as u32));
            }
            _ => res.push(c),
        }
    }
    res
}

#[cfg(test)]
mod probationes {
    use super::*;

    #[test]
    fn proba_effugium_simplex() {
        assert_eq!(ison_effuge("salve"), "salve");
    }

    #[test]
    fn proba_effugium_speciale() {
        assert_eq!(ison_effuge("a\"b"), "a\\\"b");
        assert_eq!(ison_effuge("a\\b"), "a\\\\b");
        assert_eq!(ison_effuge("a\nb"), "a\\nb");
    }
}
