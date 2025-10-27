# DDD Design

## Server location

The goal is to reduce latency for most players.

As [stebler](https://github.com/stblr) decided to stick with OVH, the following sites were evaluated:

- **bhs**: Beauharnois, Canada (near Montreal)
- **eri**: Erith, UK (near London)
- **gra**: Gravelines, France (near Dunkirk)

The following tools were used to measure latency:

- <https://check-host.net/check-ping>
- <https://viewdns.info/ping>

| Location     | `bhs.lg.ovh.net` | `eri.lg.ovh.net` | `gra.lg.ovh.net` |
| :----------- | ---------------: | ---------------: | ---------------: |
| Atlanta      |        **24 ms** |            86 ms |            93 ms |
| Coventry     |            79 ms |         **4 ms** |             6 ms |
| Johannesburg |           253 ms |       **168 ms** |       **168 ms** |
| Mumbai       |           206 ms |       **136 ms** |       **136 ms** |
| São Paulo    |       **124 ms** |           198 ms |           194 ms |
| Seattle      |        **69 ms** |           141 ms |           148 ms |
| Sydney       |       **197 ms** |           267 ms |           272 ms |
| Tokyo        |       **171 ms** |           241 ms |           248 ms |
| Warsaw       |           107 ms |            32 ms |        **26 ms** |

The **Beauharnois** site was chosen as it seems to offer the best compromise.
In particular, the Seattle and Warsaw results motivated that decision.
