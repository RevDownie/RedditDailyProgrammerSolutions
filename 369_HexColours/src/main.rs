
struct RGB
{
    r: u8,
    g: u8,
    b: u8
}

/// Convert individual RGB values to a hex string in the format '#ffffff'
///
fn rgb_hex_to_string(col: &RGB) -> String
{
    let result = ((col.r as u32) << 16) | ((col.g as u32) << 8) | (col.b as u32);
    return format!("#{:06X}", result);
}

/// Puzzle is a Simple RGB to hex converter
/// 
fn main()
{
    const TEST_CASES: [RGB; 4] = 
    [
        RGB {r: 255, g: 99, b: 71},
        RGB {r: 184, g: 134, b: 11},
        RGB {r: 189, g: 183, b: 107},
        RGB {r: 0, g: 0, b: 205},
    ];

    let hexs = TEST_CASES.iter().map(rgb_hex_to_string);
    hexs.for_each(|h| println!("{:?}", h));
}