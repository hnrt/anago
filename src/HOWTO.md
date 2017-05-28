# How to update the source file dependency in Makefile

Update OBJS1 macro and do as follows.

    sh ../support/scripts/update_dependency.sh Makefile

# How to update the localization files

0. Build tools once.

    pushd $(PROJECTROOT)/support/src
    make both
    popd

1. If one or more gettext lines were added or removed, update POTFILES in Makefile.

    sh ../support/scripts/update_potlist.sh Makefile

2. If one or more strings passed to gettext were changed, update po files.

    make po

3. Check updated strings in English po file.

    ../support/bin/linux/release/pocheck Localization/en_US/Anago.po

4. Translate po files.

    1. If translations (msgstr) should be exactly the same as labels (msgid), just copy msgid to msgstr.

        ../support/bin/linux/release/poupdate -copy Localization/xx_YY/Anago.po

    2. Otherwise, translate po file by hand.

        vi Localization/xx_YY/Anago.po

# How to build binaries

* To build the debug version, just run make or do as follows.

    make debug

* To build the release version, do as follows.

    make release

* To build both versions, do as follows.

    make both

# How to install binaries to the local computer

    su
    make install

