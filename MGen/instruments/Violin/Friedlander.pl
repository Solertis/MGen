# Main
library = Friedlander Violin 1.5 # For which library algorithm is optimized
Type = 1 # Instrument type

# Instrument parameters
n_min = G3 # Lowest note
n_max = F7 # Highest note
t_min = 100 # Shortest note in ms
t_max = 0 # Longest melody withot pauses in ms (0 = no limit). Decreases with dynamics
poly = 1 # Maximum number of simultaneous voices
#leap_t_min = 100 # Shortest note after leap in ms

# Automation parameters
CC_dynamics = 11
CC_ma = 9 # Number of CC steps to moving average (please use only odd numbers)
CC_steps = 30 # Number of CC steps in one note step (please use only odd numbers)

# Retrigger adaptor
CC_retrigger = 64 # This CC will be set before note retriger and reset soon after
retrigger_freq = 50 # Percent to use CC retrigger instead of non-legato when repeating notes
retrigger_min_len = 600 # Minimum next note length in ms to use retrigger
retrigger_rand_max = 300 # Maximum length in ms to move note end to the left in case of nonlegato retrigger
retrigger_rand_end = 50 # Maximum percent of note length to move note end to the left in case of nonlegato retrigger

# Slur adaptor
max_slur_count = 2 # Use slur for 2nd moves, but no longer than X moves
max_slur_interval = 2 # in semitones
slur_ks = 44 # Keyswitch to send to create slur

# Legato adaptor
legato_ahead = 165 # Time in ms to stretch legato notes back to cope with legato delay
max_ahead_note = 12 # Maximum chromatic interval having ahead property

# Nonlegato adaptor
nonlegato_freq = 20 # Frequency (in percent) when legato can be replaced with non-legato by moving note end to the left
nonlegato_minlen = 400 # Minimum note length (in ms) allowed to convert to nonlegato

# Transition types
vel_harsh = 118 # Velocity equal or above this value triggers harsh sustain
vel_immediate = 84 # Velocity equal or above this value triggers immediate sustain
vel_normal = 50 # Velocity equal or above this value triggers normal sustain
vel_gliss = 18 # Velocity below this value triggers glissando transition
vel_normal_minlen = 600 # Minimum note length (ms) that can have a normal or lower sustain

# Gliss adaptor
gliss_minlen = 1000 # Minimum note length that can have a gliss transition
gliss_freq = 50 # Frequency of gliss articulation in percent

# Bell adaptor
bell_mindur = 700 # Minimum note duration (ms) that can have a bell
bell_mul = 0.2-0.2 # Multiply dynamics by this parameter at bell start-end
bell_len = 30-30 # Percent of notelength to use for slope at bell start-end

# Reverse bell adaptor
rbell_freq = 80 # Frequency to apply reverse bell when all conditions met
rbell_dur = 300-1000 # Minimum note duration (ms) that can have a reverse bell - that will have deepest reverse bell
rbell_mul = 0.8-0.2 # Multiply dynamics by this parameter at bell center with mindur - with longer dur
rbell_pos = 20-80 # Leftmost-rightmost minimum reverse bell position inside window (percent of window duration)

# Vibrato adaptor
CC_vib = 1 # CC number for vibrato intensity
CC_vibf = 14 # CC number for vibrato speed
vib_bell_top = 10-90 # Leftmost-rightmost maximum vibrato intensity in note (percent of note duration)
vibf_bell_top = 10-90 # Leftmost-rightmost maximum vibrato speed in note (percent of note duration)
vib_bell_exp = 2 # Exponent to create non-linear bell shape
vibf_bell_exp = 2 # Exponent to create non-linear bell shape
vib_bell_freq = 100 # Frequency to apply vibrato bell when all conditions met
vib_bell_dur = 600-1200 # Minimum note duration (ms) that can have a vibrato bell - that can have highest vibrato bell
vib_bell = 30-90 # Maximum vibrato intensity in vibrato bell (for minimum and highest duration)
vibf_bell = 20-80 # Max vibrato frequency in vibrato bell (for minimum and highest duration)
rnd_vib = 10 # Randomize vibrato intensity not greater than this percent
rnd_vibf = 10 # Randomize vibrato speed not greater than this percent

# Randomization
rnd_vel = 8 # Randomize note velocity not greater than this percent
rnd_dyn = 8 # Randomize step dynamics not greater than this percent
