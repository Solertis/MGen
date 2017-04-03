# Main parameters
t_cnt = 10 # Maximum number of canti generated
c_len = 10 # Number of measures in each cantus. Usually 9 to 11
s_len = 5 # Maximum number of measures to full scan
min_interval = 4 # Minimum diatonic interval in cantus (7 = octave)
max_interval = 7 # Maximum diatonic interval in cantus (7 = octave)
first_note = C5 # Starting note of each cantus
last_diatonic_int = 0 # Diatonic interval between first and last note
tempo = 100-130 # Tempo range
shuffle = 1 # If you want to shuffle all canti after generation (can shuffle up to 32000 canti)
random_seed = 1 # Seed melody with random numbers. This ensures giving different results if generation is very slow.
calculate_correlation = 0 # Enables correlation calculation algorithm. Slows down generation. Outputs to cf1-cor.csv

# Rules: 0 - show only without violation, 1 - show with or without violation, 2 - show only violations
Strict                    = 0 # No violated rules
Prepared unfilled 3rd     = 1 # 3rd is unfilled, but was prepared by filling or leap before
Prepared unresolved 3rd   = 1 # 3rd is unresolved, but was prepared by filling or leap before
Two 3rds after 6/8        = 1 # Chain of two thirds in one direction after 6th or octave
Late <6th resolution      = 1 # Leap less than 6th is resolved after a second note
Leap back <5th            = 1 # Leap returns to same note (in case leap is equal or less than 4th)
Seventh                   = 1 # Seventh interval leap
Leap pre-late fill        = 1 # Melody has unfilled gaps
Many leaps                = 1 # Maximum allowed max_leaps during max_leap_steps
3rd to last is CEG        = 1 # Third to last note is not Subdominant (C E G forbidden)
Last leap                 = 1 # Last step is a leap
Tritone resolved          = 1 # Tritone is prepared by E and resolved to C
3 letters in a row        = 0 # 3 same letters (TDS) in possible chord names in a row
Unfilled leap             = 0 # Leap is not filled
Leap to leap resolution   = 0 # Resolution of leap with another leap
Two 3rds                  = 0 # Chain of two thirds in one direction
4 step miss               = 0 # 4 steps missing one of 3 letters (TDS) in possible chord names in a row

Long smooth               = 0 # Too long smooth movement
Long line                 = 0 # Too long smooth movement in one direction (linear)
Close repeat              = 0 # Two notes repeat in contact
Stagnation                = 0 # Stagnation on one note
Multiple culminations     = 0 # Melody has more than one culmination with the same note
2nd to last not D         = 0 # Second to last note is not Dominant (C E F A forbidden)
4 letters in a row        = 0 # 4 same letters (TDS) in possible chord names in a row
>4 letters in a row       = 0 # More than 4 same letters (TDS) in possible chord names in a row
5 step miss               = 0 # 5 steps missing one of 3 letters (TDS) in possible chord names in a row
>5 step miss              = 0 # More than 5 steps missing one of 3 letters (TDS) in possible chord names in a row
Late culmination          = 0 # Culmination at last three steps
Leap back >4th            = 0 # Leap returns to same note (in case leap is larger than 4th)
Many leaps+               = 0 # Maximum allowed max_leaps2 during max_leap_steps
Leap unresolved           = 0 # Leap unresolved during two next steps (movement direction does not change)
Leap chain                = 0 # Chain of two leaps in one direction
Late >5th resolution      = 0 # Leap greater than 5th is resolved after a second note
Tritone unresolved        = 0 # Tritone is not resolved correctly
Tritone culmination       = 0 # Tritone is the highest leap
3rd to last is leading    = 0 # Third to last note is leading (B forbidden)

fill_steps_mul = 2 # Multiply number of notes between leap notes to get steps for filling
max_smooth_direct = 5 # Maximum linear movement in one direction allowed (in steps)
max_smooth = 7 # Maximum linear movement allowed (in steps)
max_leaps = 2 # Maximum allowed max_leaps during max_leap_steps
max_leaps2 = 3 # Maximum allowed max_leaps2 during max_leap_steps for Many leaps+
max_leap_steps = 7
stag_notes = 2 # Maximum allowed stag_notes (same notes) during stag_note_steps
stag_note_steps = 7

# Technical parameters
midifile_tpq_mul = 8 # All notes are made whole for midi export
show_severity = 1 # =1 to show severity in square brackets in comments to notes (also when exporting to MIDI file)

# Not implemented
max_repeat_mul = 2 # Allow repeat of X notes after at least X*max_repeat_mul steps if beats are different
