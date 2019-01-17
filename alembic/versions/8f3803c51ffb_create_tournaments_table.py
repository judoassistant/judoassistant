"""Create tournaments table

Revision ID: 8f3803c51ffb
Revises: a01e986b126c
Create Date: 2019-01-17 17:40:02.992724

"""
from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision = '8f3803c51ffb'
down_revision = 'a01e986b126c'
branch_labels = None
depends_on = None


def upgrade():
    op.create_table(
        'tournaments',
        sa.Column('id', sa.Integer, primary_key=True),
        sa.Column('owner', sa.Integer, sa.ForeignKey('users.id'), nullable=False),
        sa.Column('tournament_id', sa.Integer, nullable=False),
        sa.Column('web_name', sa.String, nullable=False, unique=True),
    )

def downgrade():
    op.drop_table('tournaments')

