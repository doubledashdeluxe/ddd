%define _binary_payload w7T.xzdio
%define _rpmfilename %name.%_arch.rpm
%define __strip /bin/true
%define systemd_update_helper %_usr/lib/systemd/systemd-update-helper

Name:    ddd-server
Version: %(cat data/version.txt)
Release: 1
Summary: Double Dash Deluxe game server
License: AGPL-3.0-only
URL:     https://ddd.gg

%description
The game server for the Double Dash Deluxe mod for Mario Kart: Double Dash!!

%prep

%build

%install
install -D -m644 %(pwd)/admin/systemd/%name.service %buildroot%_usr/lib/systemd/system/%name.service
install -D %(pwd)/target/%_arch-unknown-linux-gnu/release/%name %buildroot%_bindir/%name
install -D %(pwd)/target/%_arch-unknown-linux-gnu/release/keygen %buildroot%_bindir/ddd-keygen

%post
%systemd_update_helper install-system-units %name.service

%preun
%systemd_update_helper remove-system-units %name.service

%files
%_bindir/ddd-keygen
%_bindir/%name
%_usr/lib/systemd/system/%name.service
