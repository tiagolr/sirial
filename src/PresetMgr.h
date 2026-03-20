#pragma once
#include <JuceHeader.h>

class SirialAudioProcessor;

class PresetMgr
{
public:
	struct Preset
	{
		String name;
		String category;
		const void* data;
    	int size;
	};

	String dir;
	inline static const std::array<Preset, 42> factoryPresets =
    {{
        { "-- Init --", "", BinaryData::init_xml, BinaryData::init_xmlSize },

        { "Rise", "Basic", BinaryData::Rise_xml, BinaryData::Rise_xmlSize },
        { "Fall", "Basic", BinaryData::Fall_xml, BinaryData::Fall_xmlSize },
        { "Fall And Rise", "Basic", BinaryData::Fall_and_Rise_xml, BinaryData::Fall_and_Rise_xmlSize },
        { "Alternate", "Basic", BinaryData::Alternate_xml, BinaryData::Alternate_xmlSize },
        { "Swing", "Basic", BinaryData::Swing_xml, BinaryData::Swing_xmlSize },
        { "Shuffle", "Basic", BinaryData::Shuffle_xml, BinaryData::Shuffle_xmlSize },
        { "Ping-Pong", "Basic", BinaryData::PingPong_xml, BinaryData::PingPong_xmlSize },
        { "Ping-Center-Pong", "Basic", BinaryData::PingCenterPong_xml, BinaryData::PingCenterPong_xmlSize },
        { "Haas", "Basic", BinaryData::Haas_xml, BinaryData::Haas_xmlSize },
        { "Triplet Taps", "Basic", BinaryData::Triplet_Taps_xml, BinaryData::Triplet_Taps_xmlSize },
        { "Reverse Triplets", "Basic", BinaryData::Reverse_Tripplets_xml, BinaryData::Reverse_Tripplets_xmlSize },
        
        { "One Two Three", "Mono", BinaryData::One_Two_Three_xml, BinaryData::One_Two_Three_xmlSize},
        { "Pairs", "Mono", BinaryData::Pairs_xml, BinaryData::Pairs_xmlSize},
        { "Dots", "Mono", BinaryData::Dots_xml, BinaryData::Dots_xmlSize },
        { "Faster", "Mono", BinaryData::Faster_xml, BinaryData::Faster_xmlSize},
        { "Slowing Down", "Mono", BinaryData::Slowing_Down_xml, BinaryData::Slowing_Down_xmlSize},
        { "More Shuffle", "Mono", BinaryData::More_Shuffle_xml, BinaryData::More_Shuffle_xmlSize},
        { "Tris", "Mono", BinaryData::Tris_xml, BinaryData::Tris_xmlSize},
        { "Pulse", "Mono", BinaryData::Pulse_xml, BinaryData::Pulse_xmlSize},
        { "Spaced", "Mono", BinaryData::Spaced_xml, BinaryData::Spaced_xmlSize},
        { "Pattern1", "Mono", BinaryData::Pattern1_xml, BinaryData::Pattern1_xmlSize},
        { "Pattern2", "Mono", BinaryData::Pattern2_xml, BinaryData::Pattern2_xmlSize},
        { "Pattern3", "Mono", BinaryData::Pattern3_xml, BinaryData::Pattern3_xmlSize},
        { "Pattern4", "Mono", BinaryData::Pattern4_xml, BinaryData::Pattern4_xmlSize},
        { "Trance1", "Mono", BinaryData::Trance2_xml, BinaryData::Trance2_xmlSize},
        { "Trance2", "Mono", BinaryData::Trance3_xml, BinaryData::Trance3_xmlSize},

        { "Spiral", "Stereo", BinaryData::Spiral_xml, BinaryData::Spiral_xmlSize},
        { "Dotted Seq", "Stereo", BinaryData::Dotted_Seq_xml, BinaryData::Dotted_Seq_xmlSize},
        { "Ping-Less-Pong", "Stereo", BinaryData::PingLessPong_xml, BinaryData::PingLessPong_xmlSize},
        { "Metronome", "Stereo", BinaryData::Metronome_xml, BinaryData::Metronome_xmlSize},
        { "Arrhythmia", "Stereo", BinaryData::Arrhythmia_xml, BinaryData::Arrhythmia_xmlSize},
        { "Sequence1", "Stereo", BinaryData::Sequence1_xml, BinaryData::Sequence1_xmlSize},
        { "Sequence2", "Stereo", BinaryData::Sequence2_xml, BinaryData::Sequence2_xmlSize},
        { "Sequence3", "Stereo", BinaryData::Sequence3_xml, BinaryData::Sequence3_xmlSize},
        { "Sequence4", "Stereo", BinaryData::Sequence4_xml, BinaryData::Sequence4_xmlSize},
        { "Sequence5", "Stereo", BinaryData::Sequence5_xml, BinaryData::Sequence5_xmlSize},
        { "Sequence6", "Stereo", BinaryData::Sequence6_xml, BinaryData::Sequence6_xmlSize},
        { "Sequence7", "Stereo", BinaryData::Sequence7_xml, BinaryData::Sequence7_xmlSize},
        { "Sequence8", "Stereo", BinaryData::Sequence8_xml, BinaryData::Sequence8_xmlSize},
        { "Sequence9", "Stereo", BinaryData::Sequence9_xml, BinaryData::Sequence9_xmlSize},
        { "Sequence10", "Stereo", BinaryData::Sequence10_xml, BinaryData::Sequence10_xmlSize},
    }};

	PresetMgr(SirialAudioProcessor& proc, String _dir);
	void loadPrev();
	void loadNext();
	void loadInit();
	void loadFactory(int index);
	String exportPreset();
	void load(String name, int offset);

private:
	SirialAudioProcessor& audioProcessor;
};