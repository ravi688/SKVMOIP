#pragma once

namespace SKVMOIP
{
	enum PS2Set1MakeCode : u32
	{
		SystemPower = 0xE05E,
		SystemSleep = 0xE05F,
		SystemWake = 0xE063,
		NoEvent /* = None */,
		OverrunError = 0xFF,
		POSTFail = 0xFC,
		ErrorUndefined, /* UNASSIGNED */
		aA = 0x1E,
		bB = 0x30,
		cC = 0x2E,
		dD = 0x20,
		eE = 0x12,
		fF = 0x21,
		gG = 0x22,
		hH = 0x23,
		iI = 0x17,
		jJ = 0x24,
		kK = 0x25,
		lL = 0x26,
		mM = 0x32,
		nN = 0x31,
		oO = 0x18,
		pP = 0x19,
		qQ = 0x10,
		rR = 0x13,
		sS = 0x1F,
		tT = 0x14,
		uU = 0x16,
		vV = 0x2F,
		wW = 0x11,
		xX = 0x2D,
		yY = 0x15,
		zZ = 0x2C,
		Digit_1Exclaim = 0x02,
		Digit_2AtSign = 0x03,
		Digit_3Hash = 0x04,
		Digit_4Dollar = 0x05,
		Digit_5Percent = 0x06,
		Digit_6Hat = 0x07,
		Digit_7Ampersand = 0x08,
		Digit_8Asterisk = 0x09,
		Digit_9OpenParen = 0x0A,
		Digit_0CloseParen = 0x0B,
		Return = 0x1C,
		Escape = 0x01,
		Backspace = 0x0E,
		Tab = 0x0F,
		Space = 0x39,
		MinusUnderscore = 0x0C,
		EqualPlus = 0x0D,
		OpenBracketBrace = 0x1A,
		CloseBracketBrace = 0x1B,
		BackSlashPillor = 0x2B,
		Europe1 = 0x2B,
		SemiColonColon = 0x27,
		QuoteSingleDouble = 0x28,
		Tilde = 0x29,
		CommaLessThan = 0x33,
		FullStopGreaterThan = 0x34,
		ForwardSlashQuestion = 0x35,
		CapsLock = 0x3A,
		F1 = 0x3B,
		F2 = 0x3C,
		F3 = 0x3D,
		F4 = 0x3E,
		F5 = 0x3F,
		F6 = 0x40,
		F7 = 0x41,
		F8 = 0x42,
		F9 = 0x43,
		F10 = 0x44,
		F11 = 0x57,
		F12 = 0x58,
		PrintScreen = 0xE037,
		ScrollLock = 0x46,
		BreakCtrlPause1 = 0xE046,
		BreakCtrlPause2 = 0xE0C6,
		Pause1 = 0xE11D45,
		Pause2 = 0xE19DC5,
		Insert = 0xE052,
		Home = 0xE047,
		PageUp = 0xE049,
		Delete = 0xE053,
		End = 0xE04F,
		PageDown = 0xE051,
		RightArrow = 0xE04D,
		LeftArrow = 0xE04B,
		DownArrow = 0xE050,
		UpArrow = 0xE048,
		NumLock = 0x45,
		KeypadDivide = 0xE035,
		KeypadMultiply = 0x37,
		KeypadMinus = 0x4A,
		KeypadPlus = 0x4E,
		KeypadEnter = 0xE01C,
		Keypad1End = 0x4F,
		Keypad2Down = 0x50,
		Keypad3PageDn = 0x51,
		Keypad4Left = 0x4B,
		Keypad5 = 0x4C,
		Keypad6Right = 0x4D,
		Keypad7Home = 0x47,
		Keypad8Up = 0x48,
		Keypad9PageUp = 0x49,
		Keypad0Insert = 0x52,
		KeypadDotDelete = 0x53,
		Europe2 = 0x56,
		App = 0xE05D,
		KeyboardPower = 0xE05E,
		KeypadEqual = 0x59,
		F13 = 0x64,
		F14 = 0x65,
		F15 = 0x66,
		F16 = 0x67,
		F17 = 0x68,
		F18 = 0x69,
		F19 = 0x6A,
		F20 = 0x6B,
		F21 = 0x6C,
		F22 = 0x6D,
		F23 = 0x6E,
		F24 = 0x76,
		// KeyboardExecute, /* UNASSIGNED  */
		// KeyboardHelp, /* UNASSIGNED  */
		// KeyboardMenu, /* UNASSIGNED  */
		// KeyboardSelect, /* UNASSIGNED  */
		// KeyboardStop, /* UNASSIGNED  */
		// KeyboardAgain, /* UNASSIGNED  */
		// KeyboardUndo, /* UNASSIGNED  */
		// KeyboardCut, /* UNASSIGNED  */
		// KeyboardCopy, /* UNASSIGNED  */
		// KeyboardPaste, /* UNASSIGNED  */
		// KeyboardFind, /* UNASSIGNED  */
		// KeyboardMute, /* UNASSIGNED  */
		// KeyboardVolumeUp, /* UNASSIGNED  */
		// KeyboardVolumeDn, /* UNASSIGNED  */
		// KeyboardLockingCapsLock, /* UNASSIGNED */
		// KeyboardLockingNumLock, /* UNASSIGNED */
		// KeyboardLockingScrollLock, /* UNASSIGNED */
		KeypadCommaBrazilianKeypadDot = 0x7E,
		KeyboardEqualSign, /* UNASSIGNED */

		KeyboardInternational1 = 0x73,
		KeyboardInternational2 = 0x70,
		KeyboardInternational3 = 0x7D,
		KeyboardInternational4 = 0x79,
		KeyboardInternational5 = 0x7B,
		KeyboardInternational6 = 0x5C,
		// KeyboardInternational7, /* UNASSIGNED */
		// KeyboardInternational8, /* UNASSIGNED */
		// KeyboardInternational9, /* UNASSIGNED */

		KeyboardLang1 = 0xF2,
		KeyboardLang2 = 0xF1,
		KeyboardLang3 = 0x78,
		KeyboardLang4 = 0x77,
		KeyboardLang5 = 0x76,
		// KeyboardLang6, /* UNASSIGNED */
		// KeyboardLang7, /* UNASSIGNED */
		// KeyboardLang8, /* UNASSIGNED */
		// KeyboardLang9, /* UNASSIGNED */

		KeyboardAlternateErase, /*  UNASSIGNED */
		KeyboardSysReqAttention, /*  UNASSIGNED */
		KeyboardCancel, /*  UNASSIGNED */
		KeyboardClear, /*  UNASSIGNED */
		KeyboardPrior, /*  UNASSIGNED */
		KeyboardReturn, /*  UNASSIGNED */
		KeyboardSeparator, /*  UNASSIGNED */
		KeyboardOut, /*  UNASSIGNED */
		KeyboardOper, /*  UNASSIGNED */
		KeyboardClearAgain, /*  UNASSIGNED */
		KeyboardCrSelProps, /*  UNASSIGNED */
		KeyboardExSel, /*  UNASSIGNED */
		Reserved1, /* RESERVED */
		LeftControl = 0x1D,
		LeftShift = 0x2A,
		LeftAlt = 0x38,
		LeftGUI = 0xE05B,
		RightControl = 0xE01D,
		RightShift = 0x36,
		RightAlt = 0xE038,
		RightGUI = 0xE05C,
		RESERVED2, /* RESERVED */
		ScanNextTrack = 0xE019,
		ScanPreviousTrack = 0xE010,
		Stop = 0xE024,
		PlayPause = 0xE022,
		Mute = 0xE020,
		BassBoost, /*UNASSIGNED */
		Loudness, /*UNASSIGNED */
		VolumeUp = 0xE030,
		VolumeDown = 0xE02E,
		BassUp, /* UNASSIGNED */
		BassDown, /* UNASSIGNED */
		TrebleUp, /* UNASSIGNED */
		TrebleDown, /* UNASSIGNED */
		MediaSelect = 0xE06D,
		Mail = 0xE06C,
		Calculator = 0xE021,
		MyComputer = 0xE06B,
		WWWSearch = 0xE065,
		WWWHome = 0xE032,
		WWWBack = 0xE06A,
		WWWForward = 0xE069,
		WWWStop = 0xE068,
		WWWRefresh = 0xE067,
		WWWFavorites = 0xE066
	};
}