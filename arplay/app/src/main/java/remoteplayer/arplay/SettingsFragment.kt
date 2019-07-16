package remoteplayer.arplay

import android.os.Bundle
import android.support.v7.preference.ListPreference
import android.support.v7.preference.Preference
import android.support.v7.preference.PreferenceFragmentCompat
import android.support.v7.preference.PreferenceManager

class SettingsFragment: PreferenceFragmentCompat(), Preference.OnPreferenceChangeListener {

	override fun onCreatePreferences(p0: Bundle?, p1: String?) {
		addPreferencesFromResource(R.xml.pref_settings)
		bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_server_address)))
	}

	override fun onPreferenceChange(p0: Preference?, p1: Any?): Boolean {
		val value = p1.toString()
		if (p0 is ListPreference) {
			val prefIndex = p0.findIndexOfValue(value)
			if (prefIndex >= 0)
				p0.setSummary(p0.entries[prefIndex])
		}
		else
			p0?.summary = value

		return true
	}

	override fun onPreferenceTreeClick(preference: Preference?): Boolean {
		return when (preference?.key) {
			getString(R.string.pref_key_server_address) -> true
			else -> super.onPreferenceTreeClick(preference)
		}
	}

	private fun bindPreferenceSummaryToValue(preference: Preference) {
		preference.onPreferenceChangeListener = this
		onPreferenceChange(preference,
			PreferenceManager.getDefaultSharedPreferences(preference.context)
				.getString(preference.key, ""))
	}
}
