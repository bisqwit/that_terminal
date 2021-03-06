static std::unordered_map<unsigned, std::tuple<
	const unsigned char*,
	std::pair<unsigned,bool>(*)(char32_t),
	std::pair<unsigned,bool>(*)(char32_t),
	bool /* isbold */
	>> fonts
{
    { 4*256 + 5,  { NS1 ns_f4x5::bitmap,   NS1 ns_f4x5::unicode_to_bitmap_index  , NS2 ns_f4x5::unicode_to_bitmap_index,   true}},
    { 4*256 + 6,  { NS1 ns_f4x6::bitmap,   NS1 ns_f4x6::unicode_to_bitmap_index  , NS2 ns_f4x6::unicode_to_bitmap_index,   true}},
    { 4*256 + 8,  { NS1 ns_f4x8::bitmap,   NS1 ns_f4x8::unicode_to_bitmap_index  , NS2 ns_f4x8::unicode_to_bitmap_index,   false}},
    { 5*256 + 7,  { NS1 ns_f5x7::bitmap,   NS1 ns_f5x7::unicode_to_bitmap_index  , NS2 ns_f5x7::unicode_to_bitmap_index,   false}},
    { 5*256 + 8,  { NS1 ns_f5x8::bitmap,   NS1 ns_f5x8::unicode_to_bitmap_index  , NS2 ns_f5x8::unicode_to_bitmap_index,   false}},
    { 6*256 + 9,  { NS1 ns_f6x9::bitmap,   NS1 ns_f6x9::unicode_to_bitmap_index  , NS2 ns_f6x9::unicode_to_bitmap_index,   false}},
    { 6*256 + 10, { NS1 ns_f6x10::bitmap,  NS1 ns_f6x10::unicode_to_bitmap_index , NS2 ns_f6x10::unicode_to_bitmap_index,  false}},
    { 6*256 + 12, { NS1 ns_f6x12::bitmap,  NS1 ns_f6x12::unicode_to_bitmap_index , NS2 ns_f6x12::unicode_to_bitmap_index,  false}},
    { 6*256 + 13, { NS1 ns_f6x13::bitmap,  NS1 ns_f6x13::unicode_to_bitmap_index , NS2 ns_f6x13::unicode_to_bitmap_index,  false}},
    { 7*256 + 13, { NS1 ns_f7x13::bitmap,  NS1 ns_f7x13::unicode_to_bitmap_index , NS2 ns_f7x13::unicode_to_bitmap_index,  true}},
    { 7*256 + 14, { NS1 ns_f7x14::bitmap,  NS1 ns_f7x14::unicode_to_bitmap_index , NS2 ns_f7x14::unicode_to_bitmap_index,  true}},
    { 8*256 + 8,  { NS1 ns_f8x8::bitmap,   NS1 ns_f8x8::unicode_to_bitmap_index  , NS2 ns_f8x8::unicode_to_bitmap_index,   true}},
    { 8*256 + 10, { NS1 ns_f8x10::bitmap,  NS1 ns_f8x10::unicode_to_bitmap_index , NS2 ns_f8x10::unicode_to_bitmap_index,  true}},
    { 8*256 + 12, { NS1 ns_f8x12::bitmap,  NS1 ns_f8x12::unicode_to_bitmap_index , NS2 ns_f8x12::unicode_to_bitmap_index,  true}},
    { 8*256 + 13, { NS1 ns_f8x13::bitmap,  NS1 ns_f8x13::unicode_to_bitmap_index , NS2 ns_f8x13::unicode_to_bitmap_index,  true}},
    { 8*256 + 14, { NS1 ns_f8x14::bitmap,  NS1 ns_f8x14::unicode_to_bitmap_index , NS2 ns_f8x14::unicode_to_bitmap_index,  true}},
    { 8*256 + 15, { NS1 ns_f8x15::bitmap,  NS1 ns_f8x15::unicode_to_bitmap_index , NS2 ns_f8x15::unicode_to_bitmap_index,  true}},
    { 8*256 + 16, { NS1 ns_f8x16::bitmap,  NS1 ns_f8x16::unicode_to_bitmap_index , NS2 ns_f8x16::unicode_to_bitmap_index,  true}},
    { 8*256 + 19, { NS1 ns_f8x19::bitmap,  NS1 ns_f8x19::unicode_to_bitmap_index , NS2 ns_f8x19::unicode_to_bitmap_index,  true}},
    { 9*256 + 15, { NS1 ns_f9x15::bitmap,  NS1 ns_f9x15::unicode_to_bitmap_index , NS2 ns_f9x15::unicode_to_bitmap_index,  true}},
    { 9*256 + 16, { NS1 ns_f9x16::bitmap,  NS1 ns_f9x16::unicode_to_bitmap_index , NS2 ns_f9x16::unicode_to_bitmap_index,  true}},
    { 9*256 + 18, { NS1 ns_f9x18::bitmap,  NS1 ns_f9x18::unicode_to_bitmap_index , NS2 ns_f9x18::unicode_to_bitmap_index,  true}},
    { 10*256+ 20, { NS1 ns_f10x20::bitmap, NS1 ns_f10x20::unicode_to_bitmap_index, NS2 ns_f10x20::unicode_to_bitmap_index, false}},
    { 10*256+ 24, { NS1 ns_f10x24::bitmap, NS1 ns_f10x24::unicode_to_bitmap_index, NS2 ns_f10x24::unicode_to_bitmap_index, false}},
    { 11*256+ 22, { NS1 ns_f11x22::bitmap, NS1 ns_f11x22::unicode_to_bitmap_index, NS2 ns_f11x22::unicode_to_bitmap_index, false}},
    { 12*256+ 12, { NS1 ns_f12x12::bitmap, NS1 ns_f12x12::unicode_to_bitmap_index, NS2 ns_f12x12::unicode_to_bitmap_index, true }},
    { 12*256+ 13, { NS1 ns_f12x13::bitmap, NS1 ns_f12x13::unicode_to_bitmap_index, NS2 ns_f12x13::unicode_to_bitmap_index, false}},
    { 12*256+ 24, { NS1 ns_f12x24::bitmap, NS1 ns_f12x24::unicode_to_bitmap_index, NS2 ns_f12x24::unicode_to_bitmap_index, false}},
    { 14*256+ 14, { NS1 ns_f14x14::bitmap, NS1 ns_f14x14::unicode_to_bitmap_index, NS2 ns_f14x14::unicode_to_bitmap_index, true}},
    { 14*256+ 28, { NS1 ns_f14x28::bitmap, NS1 ns_f14x28::unicode_to_bitmap_index, NS2 ns_f14x28::unicode_to_bitmap_index, true}},
    { 16*256+ 16, { NS1 ns_f16x16::bitmap, NS1 ns_f16x16::unicode_to_bitmap_index, NS2 ns_f16x16::unicode_to_bitmap_index, false}},
    { 16*256+ 28, { NS1 ns_f16x28::bitmap, NS1 ns_f16x28::unicode_to_bitmap_index, NS2 ns_f16x28::unicode_to_bitmap_index, true}},
    { 16*256+ 32, { NS1 ns_f16x32::bitmap, NS1 ns_f16x32::unicode_to_bitmap_index, NS2 ns_f16x32::unicode_to_bitmap_index, false}},
    { 18*256+ 18, { NS1 ns_f18x18::bitmap, NS1 ns_f18x18::unicode_to_bitmap_index, NS2 ns_f18x18::unicode_to_bitmap_index, false}},
    { 24*256+ 24, { NS1 ns_f24x24::bitmap, NS1 ns_f24x24::unicode_to_bitmap_index, NS2 ns_f24x24::unicode_to_bitmap_index, false}},
};
