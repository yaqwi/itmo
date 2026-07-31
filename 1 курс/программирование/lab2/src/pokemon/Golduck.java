package pokemon;

import move.physical.AquaJet;

public final class Golduck extends Psyduck {
    public Golduck(String name, int level) {
        super(name, level);
        setStats(80, 82, 78, 95, 80, 85);
        addMove(new AquaJet());
    }
}
