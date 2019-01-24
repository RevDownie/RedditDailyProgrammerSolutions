
struct RGB
{
    r: u8,
    g: u8,
    b: u8
}

/// Convert the individual RGB values to a u32 int with R in the most significant bit
/// 
fn rgb_to_rgbint(col: &RGB) -> u32
{
    ((col.r as u32) << 16) | ((col.g as u32) << 8) | (col.b as u32)
}

/// Convert int to a hex string in the format '#ffffff'
///
fn int_to_hexstring(col: u32) -> String
{
    format!("#{:06X}", col)
}

/// Convert the individual RGB values to a hex string in the format '#ffffff'
/// 
fn rgb_to_hexstring(col: &RGB) -> String
{
    int_to_hexstring(rgb_to_rgbint(col))
}

/// Convert the hex character 0-F to decimal range 0-15
/// using ASCII table ranges for 0-9 and A-F
/// 
fn hex_table(x: u8) -> u8
{
    match x
    {
        48...57 => x - 48,
        65...70 => x - 55,
        _ => panic!("Invalid hex value")
    }
}

/// Convert hex chars in the format '#ffffff' to RGB int
///
fn hexstring_to_rgb(hex: &&str) -> RGB
{
    let chars = hex.as_bytes();
    let r = hex_table(chars[1]) * 16 + hex_table(chars[2]);
    let g = hex_table(chars[3]) * 16 + hex_table(chars[4]);
    let b = hex_table(chars[5]) * 16 + hex_table(chars[6]);
    RGB {r, g, b}
}

/// Average the given RGB values together (NOTE: The puzzle says to round)
/// 
fn rgb_blend(cols: &[RGB]) -> RGB
{
    let mut sum_r = 0u32;
    let mut sum_g = 0u32;
    let mut sum_b = 0u32;

    for c in cols.iter()
    {
        sum_r += c.r as u32;
        sum_g += c.g as u32;
        sum_b += c.b as u32;
    }

    let av_r = sum_r as f32 / cols.len() as f32;
    let av_g = sum_g as f32 / cols.len() as f32;
    let av_b = sum_b as f32 / cols.len() as f32;

    RGB {r: av_r as u8, g: av_g as u8, b: av_b as u8}
}

/// Puzzle is a Simple RGB to hex converter broken into 2 parts
/// 1. Convert RGB to hex
/// 2. Average a number of hex values together
/// 
fn main()
{
    let test_cases_convert: [RGB; 4] = 
    [
        RGB {r: 255, g: 99, b: 71}, //#FF6347
        RGB {r: 184, g: 134, b: 11}, //#B8860B
        RGB {r: 189, g: 183, b: 107}, //#BDB76B
        RGB {r: 0, g: 0, b: 205}, //#0000CD
    ];

    let test_cases_blend: [Vec<&str>; 2] = 
    [
        vec!["#000000", "#778899"], //#3B444C
        vec!["#E6E6FA", "#FF69B4", "#B0C4DE"], //#DCB1D9
    ];

    // Simple RGB to hex conversion
    println!("--- Tests 1");
    let hexs = test_cases_convert.iter().map(rgb_to_hexstring);
    hexs.for_each(|h| println!("{:?}", h));

    // Blend multiple hex colours together
    println!("--- Tests 2");
    for t in &test_cases_blend
    {
        let rgbs = t.iter().map(hexstring_to_rgb).collect::<Vec<RGB>>();
        let average = rgb_blend(&rgbs);
        let hex = rgb_to_hexstring(&average);
        println!("{:?}", hex)
    }
}